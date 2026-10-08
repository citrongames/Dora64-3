## Продолжение по пользовательскому запуску061

В061 удалось поймать повторный отказ. Host/raw/latch delivery подтверждена,
причина в Source обработке пока не установлена; новые consumer snapshots
подготовлены в исходниках. Последний EXE061 их ещё не содержит.
См. [input-loss-after061.md](input-loss-after061.md).

# Пропавший игровой ввод в060: логи и дампы

2026-10-08. Main09ffb36; до начала анализа dirty только документы/receipt060.
Пользователь: игра продолжалась, Esc-меню работало, внутри игры ввод исчез;
reset не восстановил управление. Это похожий симптом прежнего057.

## Имеющиеся данные

Последний запуск logs/1791429799666/runtime.log — test-060. Два reset на
строках5600/9580, после каждого прошли cold boot и301B0/3C1B8/30324.
Рендер и звук продолжались до graphics task2550;301 resource captures
являются ресурсами, а не снимком состояния N64-контроллера.
CrashDumps/Dora64-3-test-060.exe.24792.dmp создан08.10.2026 09:24:10 Asia/Omsk.
SHA256 fb4ada269fc4b98cc526246a6def2bc9d599919e849dbb060fdb0ef61132e1e2.
Флаги0x121,27 threads,1271 memory ranges. Прочитан локальным DbgHelp с PDB060,
игра/renderer/GPU и автоматические тесты не запускались.

Host globals: input_enabled1, max_controllers4, menu_open0,
game_release/ui_release0, held_keys/held_pad нулевые, focus1 и controller1.
SI binding801AA668/message1 установлен. Эти значения — момент закрытия,
не история нажатий во время зависшего ввода. Дамп НЕ содержит RDRAM по
events_context.rdram=1ABA3B10000: status/mask/pad/latches недоступны.
По нему нельзя подтвердить mask801A48B4 или место потери конкретного нажатия.
Прежний057 query retry/per-control quarantine остаётся в коде; не утверждаем,
что текущий отказ воспроизведён или устранён этим старым исправлением.

## Подтверждённый отдельный краш закрытия

ExceptionC0000005 execute address1AB888E0870. Exception-context stack указывает
на RT64::ApplicationWindow::sdlEventFilter+1A; событие SDL0x606 — удаление
joystick device при завершении SDL/controller. Listener уже уничтожен.
ApplicationWindow destructor не снимает установленный SDL filter. Main
после возврата recomp::start вызывает shutdown_live_io/SDL_Quit, и SDL снова
использует сохранённый userdata. Это объясняет crash dump после живого лога,
но не доказывает причину предшествующего пропавшего игрового ввода.

LiveContext::shutdown теперь восстанавливает предыдущий SDL filter ДО app.end,
только если текущие callback/userdata принадлежат нашему appWindow. Новый владелец
filter не перезаписывается. Reset не снимает обработчик живого renderer.
Dependency checkout/pins не изменены. Четыре host units с первоначальным
вариантом диагностического лимита скомпилированы Linux C++20, exit0; итог061
использует state changes+редкий heartbeat без этого раннего лимита записей.

## Диагностика061

controller-input.jsonl в state текущего запуска сохраняет presence mask,
четыре raw OSContPad (buttons/sticks/error), query type/status_error,
port0 held/pressed/valid, host buttons/sticks/enabled, focus/menu/UI gate.
На reset сначала сохраняется старое состояние до очистки RAM; boot начинает
новый session ID, после настоящего30324 read observer в30380 видит результат.
Запись только при изменении плюс каждые300 опросов, без лога каждого кадра.
Счётчик опросов позволит отличить остановку чтения от нулевых/ошибочных данных.
Никаких fake connections/buttons, обхода locks или изменения input поведения.
Новые observer hooks находятся в host trace, canonical CPU generation
startup-menu-reset-v1 пригодна при совпадении inputs; audio-rsp-v1 прежний.
Пользователь прямо запросил061 для проверки с этим логом. Commit/push не разрешён
до результата. Код отказа ввода не объявляется исправленным.

## Предыдущий дамп и итог061

058/25476 dump также содержит C0000005 execute heap pointer, а stack thread28360
имеет RT64::ApplicationWindow::sdlEventFilter+1A. SHA256 дампа:
f5c871c17ab4b917efdeb859501bd22f27772eca0791d2ff46309928ecb9463d.
Использован сохранённый PDB058. Это подтверждает pre-existing shutdown bug,
а не связывает пропавший ввод с новым Reset. Внутренний RDRAM не сохранён
и в этом дампе; отказ управления всё ещё требует новых controller snapshots.

По прямой команде пользователя собран и упакован Windows061 из dirty snapshot
поверх09ffb36, без commit/push. EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-061.exe.
SHA256: b704dab7614044d5ab6082bd6d51ded4d831a10097e221d28394a123ed039a57.
MSVC RelWithDebInfo configure/compile/link exit0. CPU startup-menu-reset-v1 и
RSP audio-rsp-v1 переиспользованы по совпавшим inputs; обновлены8 staging files,
полный архив исходников не создавался. PDB/EXE060 и receipt060 сохранены.
Все195 прежних файлов вне logs сохранили хэши. Игра/тесты/renderer/GPU не запускались.
controller-input.jsonl в logs/<run>/state пишет host+guest buttons/status/latches
на старте, перед reset, при изменениях и каждые300 опросов. Это диагностическая
сборка для ещё НЕ установленной причины060 input loss. Отдельный доказанный
SDL shutdown use-after-free исправлен: callback снимается до уничтожения RT64.
Пользователь проверяет с логом; коммит/пуш после результата. Разбор —
docs/input-loss-after060.md. Старые060 документы ниже являются историей.

