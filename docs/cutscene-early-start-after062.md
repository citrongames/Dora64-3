## Пользовательская проверка063 и разрешение Git (2026-10-08)

Пользователь сообщил «вроде сработало, коммит и пуш» после063. Это его
подтверждение ранее описанного отказа раннего Start на заставке и прямое
разрешение сохранить текущий этап в main и отправить origin/main. Не считать
все30 сцен проверенными в игре: остальные аналоги установлены статически.
Последний EXE остаётся063, SHA256 b5ca61c393c76045457239ebe2a19e061cafc5d2e1d245c622a646453c401c27.

Сохраняемый этап включает ранний Start с owner guards/reset cleanup,
диагностику input/Source consumers, доказанную SDL filter teardown правку,
смысловые pause/resume имена и честные061..063 build/investigation receipts.
ROM, извлечённые ресурсы, generated game C, EXE/PDB, saves и реальные run
logs/dumps остаются вне Git. Код после063 не менялся; новый билд не требуется.
Агент автоматических тестов и game/emulator/renderer/GPU не запускал.
Правило сохраняется: после завершённых изменений кода сразу билд; Git после
пользовательской проверки и соответствующего разрешения.

## Test063 готов: сохранение раннего Start при инициализации заставки (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-063.exe.
SHA256: b5ca61c393c76045457239ebe2a19e061cafc5d2e1d245c622a646453c401c27.
Причина установлена по пользовательскому062 запуску: реальный Start принят
на tick1, затем начальный pause(7) опять остановил поток пропуска; однократный
requested flag блокировал повторные нажатия. После natural completion ввод
на следующих экранах работал. Все30 scripts350..379 имеют такой initializer.

Shared Source adapter сохраняет только этот принятый ранний запрос после
его точной инициализации, с script-buffer/cursor/tick/overlay-generation
guards и очисткой pending при reset. Программа пропуска исполняется обычным
interpreter на следующем Source update; buttons/waits/later pauses и
однократность не меняются. Late Start остаётся прежним. Диагностика сохранена.
Подробности: [cutscene-early-start-after062.md](cutscene-early-start-after062.md).

Новая canonical CPU generation startup-cutscene-start-after062-v1 создана
из-за новых смысловых pause/resume names,4778 функций/44 SDK/5 hardware,
0 pending; ABI/Original C сохранён. Audio-rsp-v1 прежняя. Windows MSVC
RelWithDebInfo configure/compile/link exit0, около50 секунд; упаковка exit0.
Скопированы18 изменённых staging files и необходимая новая generation;
полный source ZIP не создавался. PDB/EXE062 и receipt062 сохранены.
Все201 старых файлов вне logs сохранили размер/mtime, DLL хэши совпали.
ROM/settings/saves не менялись. Тесты и игра/emulator/renderer/GPU не запускались.
Игровой результат063 требует пользовательской проверки; commit/push нет.

# Ранний Start терялся при инициализации потока пропуска заставки

2026-10-08. Пользователь воспроизвёл в062 сразу после запуска, без reset;
после естественного окончания заставки ввод снова работал. Main09ffb36,
предыдущие незакоммиченные060/061/062 изменения сохранены.

## Доказанная последовательность

logs/1791433526009, executable062, одна сессия/3650 schema2 snapshots.
Первый script-input: poll250/source tick1, pressed=latched=selected=Start4096,
start_requested80218B00=0, skip-thread7 command80302C54/opcode3, paused=0.
Source-complete того же tick1: requested=1, command80302C5A/opcode6, paused=1.
Всего73 Start edges (включая первую) достигали E4484, но requested=1 оставался и paused=1.
Последняя paused snapshot tick798/poll2639. Script updates действительно
продолжались. На tick816 interpreter завершился, начал штатный переход;
позднее обновлялся scene34 file menu, что совпадает с наблюдением пользователя.

Exact Original E4484 принимает Start ДО исполнения команд: при8B00=0
устанавливает8B00=1 и снимает paused+A записи7 по802183CA. Затем начальный
opcode3 pause(7) опять ставит этот paused flag, cursor проходит separator1
и остаётся у первой команды пропуска. Повторный Start не помогает из-за
однократного8B00 gate. Natural opening script позже делает resume(7), поэтому
сценарий доигрывает, а следующие экраны нормально обрабатывают кнопки.
В прежних docs/input-loss-after061.md ошибочно называли802183CA wait:
это paused+A, тогда как wait находится+6 (802183C6); термин исправлен.

ROM table801FAC90 содержит30 script entries350..379; opening —376,
ROM189F30..18AEB0. ВСЕ30 начинают поток7 шаблоном3,7,1. Таким образом,
причина общая для этого семейства, а не связана исключительно с Reset,
SDL focus, отсутствием контроллера или native frame rate.
Exact body hashes, ROM identity, headers и timeline:
[cutscene-early-start-after062.json](../research/cutscene-early-start-after062.json).

## Исправление и границы

cutscene_input сохраняет только настоящий ранний Start на entry E4484,
когда запрос ещё не принят и cursor7 стоит на header[7] перед3,7,1.
После завершённого Original Source callback снимает именно начальный
paused flag, перекрывший уже принятый запрос. Не создаёт кнопок, не снимает
глобальную блокировку управления и не повторяет script commands. Программа
пропуска исполняется обычным interpreter на следующем Source update;
8B00 однократность, waits, дальнейшие pause commands, gameplay/menu input
сохраняются. Поздний Start вообще не входит в этот special case.

Владелец запроса содержит RAM pointer, script buffer, начальный cursor,
Source tick и overlay generation. При completion проверяются тот же владелец,
cursor+6, request=1, paused=1, wait=0 и незавершённый thread. Scene change,
повторная initialization, reset или завершение отменяют несоответствующий
запрос. On_init очищает pending до новых guest threads. Runtime/dependency
checkout/pins, ROM, generated C вручную и зеркало sources/ не изменяются.

Host hooks используют существующие trace и Source completion boundaries;
Original, Display и Native/Manual проходят один путь. В runtime.log при
фактическом удержании запроса будет Cutscene Start: retained request after
skip-thread initialization; schema2 получит early-start-retained snapshot.

Pause/resume Original functions закреплены смысловыми именами в каталоге с
прежним ABI; description E4484 уточнён. Поэтому для063 создаётся новая
canonical CPU generation startup-cutscene-start-after062-v1, audio-rsp-v1
прежняя. Только статический ROM/log/source анализ и необходимая генерация;
автоматических тестов, game/emulator/renderer/GPU запуска нет. После всех
изменений сразу Windows build063 по действующему разрешению пользователя.
Игровая проверка fix пользователем ещё не выполнена. Commit/push нет.
