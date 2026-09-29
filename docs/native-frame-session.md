# Native FPS: переход режима и источник кадров

2026-09-29, продолжение native-presentation.md, база fbdd4d6.
CPU hooks остаются v64; генерация игрового кода не менялась.

## Что подключено

FrameProducer запускается в init_events после успешной подготовки gfx и VI
и останавливается с join перед join_event_threads. Это один поток runtime,
который в Original спит без deadline. Он только посылает FFFFFF03 через ранее
проверенную retained external queue: guest RAM, VI/AI, игровой callback и GPU
не вызываются из этого потока. Ошибка выделения памяти очередью — явный fail-fast,
а не бесконечный retry. Лимитов длительности сессии/числа кадров не добавлено.

При активном FrameBridge поток ждёт steady_clock deadline. При занятом mailbox
или следующем банке он ждёт изменения состояния; constant polling и catch-up
пачки кадров отсутствуют. Получение сообщения, освобождение банка, смена FPS,
leave и reset будят ожидание. Номер изменения считывается ДО проверки готовности;
проверка номера и засыпание защищены одним mutex. Завершение между проверкой
готовности и фактическим ожиданием не теряется. Stop token прерывает ожидание,
включая случай уже отправленного и ещё не принятого сообщения.

При in-process reset существующий producer_mutex исключает отправку на время
сброса bridge. Очереди очищаются после него прежним runtime путём. Поток остаётся
спящим до нового enter. Serial/epoch не возвращаются к нулю.

## Барьер чтения предыдущего кадра

Одной проверки native-active недостаточно: renderer мог пройти её и ещё не начать
читать framebuffer. PresentationGate атомарно разрешает начало Original-read
либо закрывает допуск для native handoff. Lease держится до конца настоящего
ScreenUpdateAction worker; mutex не удерживается во время чтения/отрисовки.
Уже начавшийся read учитывается счётчиком, поздний отклоняется. Native present
проходит gate по собственному FrameTicket, иначе ожидание своего завершения
заблокировало бы выход из режима.

Gate распространяется и на непериодический Original screen update; native entry
не может пересечь этот путь. В Dora3 сейчас нет caller донорского explicit CPU
notify, но при его будущем добавлении он не обойдёт барьер. Periodic permit
освобождается также при отказе gate. Native ownership учитывает резервирование
gate ещё ДО первого native кадра, поэтому VI не накапливает новые старые запросы.

FrameSession выполняет Original → Acquiring → Native → Draining → Original.
begin требует от scene adapter проверенного окончания исходных графических задач
и правильного следующего банка. Затем закрывает gate. poll активирует bridge только
после окончания Original-read. Между begin и возвратом Original вызывающий код
обязан запретить исходное построение кадров и смену ресурсов/сцены/VI mode.
Это неблокирующий протокол: нельзя ждать renderer, захватив guest scheduling token.

leave отменяет новые native wake, но не очищает занятые банки. Gate открывается
только после scheduler/game/host-read подтверждений. Отмена до активации безопасно
возвращает Original. Если существующая обработка паузы уже выключила bridge,
poll переводит session в Draining. Reset допускается после runtime barriers;
попытка сброса gate с незавершённым read отклоняется.

## Проверено

tools/check_native_session.py с ASan/LSan и TSan проверяет настоящий поток producer
на synthetic sender/consumer. Удержанный read задерживает enter; late read не
начинается; два занятых банка и retained mailbox не создают поток сообщений;
частичное completion не открывает банк; полное будит producer. Проверены смена
FPS с устаревшим wake, pause/leave/reset, остановка с ожидающим сообщением и restart.
Небольшие watchdog timeout существуют только в тесте, не в игровом runtime.

tools/check_native_present.py теперь вызывает begin изнутри fake update_screen
НАСТОЯЩЕГО извлечённого worker. На этой точке poll остаётся Acquiring; после
возврата worker становится Native. ASan/LSan и TSan пройдены. Настоящий renderer
не запускался. Original bridge/pump checks и прежние CPU framebuffer/VI проверки
также выполнены; TSan предупреждает об unsupported atomic_thread_fence в pinned
concurrentqueue при VI check, поэтому это не доказательство всех гонок библиотеки.

Linux/Windows скомпилированы. При первом интеграционном build выявлена зависимость
runtime producer header от recomp.h, отсутствующего в include path ultramodern.
Исправлено разделением интерфейса: runtime header использует forward declaration,
guest bridge включён только в producer.cpp. Донорские include paths не расширялись.
Отчёты: research/native-session-*-check.json. Android и визуальные тесты не выполнялись.

## Что ещё не включено

Ни одна игровая сцена пока не вызывает FrameSession.begin: producer запускается,
но остаётся спящим. Manual/test-024 ещё используют прежний RT64-путь.
Нельзя объявлять готовым игровое переключение только по наличию FrameSession:
реальный scene adapter должен подтвердить legacy task retirement, остановить
Original builds на Acquiring/Draining, продолжать принимать completion и вызывать
poll. Также нужны actual settings routing и преобразование update/fade первой сцены.

Следующая цель — маленькая scene34. Проверено по оригинальным инструкциям:
init801C0184 пишет divider3 в801A39DC; setup801C0014 ставит hold counter801C68D0=60;
update801C0088 уменьшает его по игровым тикам, а transition801BFDE0 ставит другой
counter801C68CC=16 и командует белый fade. При повышении частоты эти интервалы
должны остаться исходной длительности. init/texture load не должны повторяться
на каждый дополнительный кадр. Чистое рисование и ввод также требуют аудита:
draw801C012C выбирает image/controller branch, а update вызывает input/repeat helpers.
Полный graphics_frame_build меняет ещё801A48E0; просто чаще вызывать его без
ограничений на фазу перехода нельзя. Фиксация адресов/хэшей: native-session-analysis.json.
