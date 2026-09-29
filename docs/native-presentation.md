# Native FPS: показ кадра и окончание чтений

2026-09-29, генерация v64, продолжение native-graphics-pump.md.
Путь подключён к runtime и скомпилирован, но ни одна игровая сцена ещё не входит
в native-режим. Test-024 остаётся последней переданной сборкой; настоящего HFR
в ней нет. Это проверка инфраструктуры, не результат визуального теста.

## Подтверждённая причина

Исходная 80026D40 отправляет game completion ДО чтения task+0C и SwapBuffer.
Получение сообщения игрой само по себе не разрешает перезапись банка: scheduler
может ещё дочитывать задачу. Кроме того, runtime должен закончить чтения framebuffer
для показа. SP/DP после разбора display list этого последнего условия не доказывают.

В 8002F518 инициализируются два банка 8010E310/8013FA38, шаг31728.
bank+C0 — сообщение типа2; bank+E0 — типа4. Обычный graphics_frame_build8002FF64
передаёт a1=40 в task_submit8002FE20, выбирая именно+C0. Ранее synthetic pump test
вызывал submit с a1=0: это проверяло другой вариант сообщения, а не обычный кадр.
Проверка исправлена, оба типа теперь явно различаются.

Framebuffer-пара: 803B5000/803DA800. Инициализатор кладёт их в bank+100/+104,
меняя порядок для второго банка. Эта реализация принимает только исследованную
пару; она не заявляет поддержки произвольного guest framebuffer.

## Реализация

PresentLedger содержит ровно два слота. Каждый связывает копию FrameTicket с
адресом задачи, framebuffer и completion message. Bank и framebuffer нельзя
занять повторно до трёх подтверждений:

1. Исходный scheduler завершил последнюю работу с задачей — hook80026DBC, после
   SwapBuffer, при v0=1. Здесь создаётся запрос показа.
2. Исходный игровой loop принял type2 — hook80027A6C. Следующий delay slot ещё
   записывает уменьшенный outstanding counter; hook не вызывает SDK/yield,
   поэтому новый guest build не может опередить эту запись.
3. Renderer вернулся из update_screen, закончив чтения guest RAM для этого показа.

Первые два условия вместе подтверждают GuestTask, третье — HostRead исходного
FrameScheduler. Порядок прихода подтверждений не фиксирован. HostRead содержит
полный epoch/serial, поэтому старое подтверждение не освобождает новый кадр.
Guest message остаётся оригинальным адресом: его хранение не переиспользуется,
пока сообщение не принято. Повторная доставка уже принятого guest сообщения
не является возможностью штатной очереди; произвольное дублирование не моделируется.

Регистрация находится перед SendMesg8002FF50 и выполняется только в native
FrameBridge callback. На этой точке delay slot ещё НЕ записал framebuffer в task+0C:
используется v0, а не старое значение памяти. Проверяются тип1, flags63, game queue
801A99E0 и type2. Неподдержанная публикация приводит к явной ошибке, а не потере кадра.

Framebuffer-ready80026CEC пропускает исходный VI equality gate только для
зарегистрированной native-задачи. Все остальные задачи сохраняют исходную проверку.
Новые native requests идут через отдельный scheduler pump; VI/AI не ускоряются.

Backend копирует настройки следующего VI и поле под message_mutex+regs_mutex,
но origin берёт из framebuffer конкретной задачи. Формула совпадает с pinned
events.cpp::update_vi, включая black/repeat-line. VI state не переключается,
VI/audio counters не увеличиваются. Копия помещается в ScreenUpdateAction.
Gfx task enqueue в osSpTaskStartGo и native present enqueue выполняются одним
guest scheduler thread: их порядок сохраняется в producer subqueue runtime.
Завершённый разбор также предшествует DP, поэтому present не опережает свой parse.

Обычные периодические запросы VI при native-владении не создаются. Ранее созданные
отбрасываются в gfx worker по признаку владения и generation: запоздавший старый
запрос не оживёт даже после возврата в Original. Permit периодического показа
освобождается и при обработке, и при таком отбрасывании.

В LiveContext интерполяция RT64 выключается до разбора первого native workload.
Пока остаются native-кадры, она не включается обратно. После выхода и освобождения
кадров восстанавливается выбранная политика. Сами настройки пользователя не меняются.

## Граница гарантии памяти

В закреплённом RT64 State::updateScreen копирует нужные CPU framebuffer bytes в
present.storage; дальнейшая present queue использует собственное хранилище.
Возврат update_screen здесь означает окончание этих CPU-чтений, НЕ GPU idle.
Другие lifetime-механизмы RT64/порта сохраняются; это не универсальный GPU fence.
PresentLedger не выделяет память на каждый кадр, outstanding native requests
ограничены двумя занятыми банками. Полный runtime reset очищает ledger только
после существующих остановки guest и SP/gfx barriers. Обычный leave ничего не стирает.

## Проверки

- tools/check_native_present.py: ASan/LSan и TSan; все шесть порядков событий,
  ранний/повторный/устаревший HostRead, два занятых банка, конкурентные game/host
  подтверждения. Исходный update_vi извлекается из pinned runtime как независимый
  эталон:16 сочетаний field/black/repeat-line/framebuffer.
- Тот же стенд исполняет извлечённые настоящие native enqueue и screen worker.
  Renderer/queue endpoint подменены: проверены копирование VI, ошибка отсутствующего
  mode, отказ enqueue, HostRead после чтений, поздние periodic requests и возврат
  в Original. Настоящий GPU здесь не запускается.
- native-graphics-pump-check:11 оригинальных функций и реальная adapted queue;
  native ready при несовпадении current/next, регистрация до SendMesg, present
  после swap, ожидание HostRead. Незарегистрированные задачи сохраняют VI gate.
- native-frame-bridge-check: оригинальный loop принимает настоящее сообщение+C0,
  освобождает ledger через установленный type2 hook и уменьшает outstanding.
- Проверки исходного scheduler и frame lights повторены из-за новых hooks.
- Linux и Windows собраны без запуска. Отчёты research/native-present-*.json;
  Android этой итерацией не проверялся. Первая игра и32 dependency checkouts неизменны.

## Что ещё обязательно до включения

Нет producer timer, переключателя игровой сцены и переведённого scene consumer.
Без этого наличия hooks недостаточно для native FPS. Следующая интеграция должна:

1. Установить безопасную границу входа: закончить исходные задачи И уже выполняемый
   periodic update_screen; один snapshot active/epoch не останавливает чтение,
   которое началось до входа. Нужен явный handshake со стороны gfx worker.
2. Привязать Manual deadline к producer lifecycle, паузе, resume и завершению.
   После leave дождаться всех native задач, прежде чем менять сцену/VI mode или
   пускать исходное построение кадров в те же банки. Original VI сейчас продолжает
   вызывать исходный game loop; native scene adapter ещё должен разделить эти ветви.
3. Для первой исследованной 2D-сцены сохранить side effects phase1, original bank
   selector и outstanding counter. Перевести fade/update на elapsed time, не
   просто чаще вызывать неизменённый update. Затем добавить диагностику update/build/present.

Отдельно остаётся перевод движения, камеры, физики, скриптов и анимационных событий
для игровых сцен. Новый пользовательский EXE до полного рабочего пути не выдаётся.


## Продолжение: gate и producer подключены

Требования к host-read handoff и lifecycle timer реализованы следующим этапом:
[native-frame-session.md](native-frame-session.md). Игровой scene adapter ещё
не включает этот протокол; ограничения legacy boundary и scene conversion остаются.
