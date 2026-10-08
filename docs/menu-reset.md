# Reset game в меню Dora64-3

2026-10-08. База main57106a7, последняя Windows-сборка test-059.
Запрос: расположение и подтверждение как в Dora64, сброс внутри игры без
перезапуска приложения/окна. Эталон только прочитан: Dora64@680d81f,
src/system_overlay.cpp, drawMenu. Этот проект и sources/ не изменялись.

## Поведение и интерфейс

В подвале Continue / Reset game / Exit game; начальное смещение330 вместо220,
кнопки100 с тем же масштабированием по высоте720p. Центральное окно
«Reset game?» содержит «Return to the opening screen?», приглушённое
«Unsaved progress will be lost.», Reset / Cancel. Cancel только закрывает
подтверждение; Reset закрывает меню, отменяет захват привязки и публикует
атомарный запрос. Существующая изоляция удержанных клавиш/кнопок сохранена.

Перезапускается внутренняя игровая сессия с оригинального entrypoint.
Окно, renderer Application, настройки/FPS, контроллер и аудиоустройство
сохраняются; перезапуска процесса, SDL_Quit/SDL_Init и повторного создания
окна нет. Runtime сохраняет host EEPROM buffer и механизм сохранения;
игра штатно читает записи заново. Несохранённое игровое состояние теряется.

## Подтверждение по ROM и оригинальному коду

ROM из testbuild прочитан без изменения, SHA256:
410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
В resident dora3_game_message_loop8002787C label800278E0 — начало следующего
приёма сообщения, после возврата предыдущих callbacks, вне native build и
simulation scopes. ROM offset2CE0:3C04801B; следующие248499E0/27A50010/
0C0160F8/24060001 задают очередь801A99E0, stack+10 и blocking osRecvMesg.
Пауза8002790C и завершение графической задачи80027A6C возвращаются сюда
через08009E38. Native reserved-message и frame paths используют ту же метку.
Хук закреплён в tools/generate_startup.py с существующей проверкой ROM word,
не в сгенерированных файлах. Название исходной функции уже есть в каталоге.

## Порядок внутреннего сброса

1. game_reset.cpp на этой границе отменяет будущие Native wakes через leave.
   Ожидание обычного scene/resource transition намеренно не используется:
   при entry-pending и паузе ещё могут существовать старые guest completions.
2. recomp_request_game_reset из закреплённого runtime завершает все guest
   threads кооперативно. Supervisor ждёт их завершения и отключает таймеры.
3. Адаптер events синхронизируется с текущим VI tick и приостанавливает VI.
   SP и graphics очереди используют по одному explicit producer token под
   mutex: reset sentinel упорядочен после всех старых enqueue, включая
   enqueue из других потоков. Сам mutex вокруг implicit producers этого
   не гарантирует. Ошибка enqueue не маскируется успешным reset.
4. После SP/gfx barriers LiveContext сбрасывает RSP/RDP session, DMEM/IMEM,
   регистры, frame lights, resource retirement и старый queued PCM. Host
   render queues/counters и устройства сохраняются. Periodic present gate
   и VI retrace divider очищаются после барьера.
5. Существующий reset_message_queues callback очищает Native ledger, frame
   bridge/session, model/camera/player/enemy/minigame/UI/texture timelines
   и старые сообщения до обнуления RAM. Epoch/serial не откатываются.
6. on_init_callback очищает overlay activity (с сохранением монотонного
   next generation), аппаратное SI/PIF/SP/DP состояние, pending music и
   диагностику. Runtime загружает resident ROM и вызывает80025C00 заново.

Original, Display и Native/Manual используют один Reset-хук; он находится
до игрового pause gate и не требует нового UI FPS режима. Запрос во время
загрузки ждёт возврата в общий цикл. Reset не предназначен для восстановления
из уже зависшего/аварийно завершившегося guest thread.

## Выполненная валидация и сборка

Только чтение ROM/статический анализ, необходимая генерация и компиляция.
Новая CPU generation: build-local/startup-menu-reset-v1;4778 функций,
44 SDK замены,5 аппаратных адаптеров,0 pending. Audio generation прежняя.
Скомпилированы9 translation units: game_reset, startup_runtime, live_graphics,
live_io, port_menu, n64_hardware, generated funcs_0, adapted events и mesgqueue.
Все завершились exit0 (Linux C/C++20, объектные файлы; без линковки EXE).
Локальные compile logs: build-local/menu-reset-compile/*.compile.log.
git diff --check чистый. Тесты/CPU checks и игра/renderer/GPU не запускались.
Игровая работа Reset, повторные сбросы и визуальное соответствие требуют
проверки пользователя; компиляция этого не подтверждает.

Новый Windows EXE не собран: команды на билд не было. При следующем билде
использовать новую CPU generation и актуальный runtime adapter; прежняя
startup-sprite-edges-after058-v1 не содержит Reset-хука. Старые EXE, ROM,
настройки, saves и DLL в testbuild не изменялись.
