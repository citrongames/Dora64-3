
## Сборка077 завершена

Инкрементальный MSVC RelWithDebInfo configure/compile/link: exit0,17.375с.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-077.exe.
SHA256: 165c529caff91d7dbf083fe0c49d996cb5d4e135bccff34cf3db0c8e54d4948c.
DORA3_DIAGNOSTICS=OFF, clean CPU generation diagnostics=false/profiling=false;
DORA3_PROFILE отсутствует в compile definitions. Все243 старых файла вне logs
сохранили size/mtime; общиеDLL совпали по хэшам упаковщика. ROM/settings/saves
не переписывались. Старые076 EXE/PDB/receipt сохранены. Новых игровых измерений
нет; успешная компиляция не подтверждает FPS. Commit/push не выполнялись.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 24.125 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-076.exe.
SHA-256: `3f70a421d6c5cde13590958ceba56fd7aa788984288038e2c7b1707203624d46`.
Все 240 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
FPS измеряет пользователь; автоматические журналы и профилировщик выключены.
Commit/push не выполнялись.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 57.688 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-075.exe.
SHA-256: `4a53b62935f1c3e6d5e7cb882f05bc86f802cf2f395c69537f03f29611f36534`.
Все 237 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
FPS измеряет пользователь; автоматические журналы и профилировщик выключены.
Commit/push не выполнялись.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 17.047 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-074.exe.
SHA-256: `1d027fcd9b924e98fdab02063da063a4f1e60a11d1fdf6aaa7bdfdf5506ec029`.
Все 234 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 27.0 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-073.exe.
SHA-256: `10a1868e14b15a88a3dd03ff6bf9e318ddd5c684463738aa0cd5bde10a7a6b2d`.
Все 231 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.

##072:5x32FPS;073 фиксирует точную причину full-pool subdivision

Native common около516 calls/сек против Original20 при сопоставимом/большем
числе actors. Общее Source gameplay в Original около7мс/update. Лимит кадров
не объясняет просадку. Нужны причины min-limit и размеры slices;073 добавляет
эти counters и thread CPU durations, не меняя time/AI/contact/birth/clip rules.
Immutable membership predicates также переведены на существующий индекс.
Детали: docs/performance-timeline-partitions.md. Один user замер5x+пауза,
затем исправлять конкретный измеренный источник дробления. Git после проверки.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 18.547 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-072.exe.
SHA-256: `e481d5549ac9468966b302c266b4fed6e6c3d398fe9f8ef7338237170849f96f`.
Все 228 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.

##071: измерены Native dispatcher/identity расходы; targeted072

На5x за17.1с:10.8млн preview calls,36.7млн eligibility calls,14.5млн identity calls.
По выборке preview self примерно500мс/сек, identity inclusive256мс/сек;
вложенные оценки не складывать. Найдены полный site dispatch для каждого
E8E7C slot exit и поздние site guards; immutable definition поиск линейный.
072 направляет прежние exit handlers напрямую, проверяет site раньше live,
индексирует только immutable records. AI/contacts/time/cleanup не урезаны.
Детали и ограничения: docs/performance-native-dispatch.md. FPS требует user run.
Commit/push не выполнять без пользовательской проверки и команды.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 27.625 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-071.exe.
SHA-256: `1e92e09247d4ebeb4e70a46fb802ba4185689c041ee752ca7d509c6efb49c511`.
Все 225 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.

##070:3x улучшилось,4x/5x просадки;071 только уточняет common pool timings

Замер070 локализовал оставшийся расход в common pool self489–632мс/second
при5x. Lookup rates40–54мс, clip2–3мс. Для отделения Native hooks/axis/identity/
ownership/overlay checks и model clocks подготовлен071 без игровых изменений.
Подробности: docs/performance-common-pool-profile.md. Не делать новый FPS fix
на предположениях. Commit/push после пользовательской проверки и команды.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 26.891 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-070.exe.
SHA-256: `d35c6a88741f2176db1af0729b240998fbbf05c00793e8af4bef9f9a20fcbad8`.
Все 222 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.

## Профиль069: Native object timeline и повторный model-rate поиск;070

Лес фей:2x60FPS, на4.1x один кадр за25.9318с. Object timeline25.885с,
self16.7212с; common pool6746 вызовов. GPU0.947мс. Детали и ограничения
измерений: docs/performance-model-rate-plan.md, research/test069-performance-analysis.json.
Сформирована boundary-local model-rate table с прежними predicates и priority,
без изменений AI/contacts/elapsed time/clip edges; добавлены три уточняющих
таймера. Причина провала локализована в Native timeline; точная доля lookup
и результат070 требуют пользовательского профиля. Compile не является FPS proof.
Commit/push не выполнять до проверки и явной команды.


## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 55.531 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-069.exe.
SHA-256: `b929d6da4e129935b54515d534ad6677dad9d945bdeb9aa7d25ce6df198df989`.
Все 219 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Причина падения FPS ещё не установлена: требуется пользовательский профиль.
Commit/push не выполнялись.

## FPS после068: пользователь требует измерений, профайлер069 (2026-10-08)

068 без замеченных вылетов; action levels/Fairy Forest на3x..5x резко падают
до5–10FPS и ниже,1x/2x нормально, города на5x стабильны, пауза восстанавливает FPS.
Предположительные оптимизации этого этапа отменены; код возвращён к068.
Причина не установлена. Пользователь прямо разрешил profiler build и потребовал
сначала доказанную причину. Правило сохранено в AGENTS.md.
Добавлены только наблюдательные scopes, Source wrappers и существующие RT64
CPU/GPU timestamps;14 игровых bodies совпадают с068. Генерация --profile,
потом один Windows069, без запусков/тестов. После пользовательского лога
сначала анализ измерений, затем целевой fix. Commit/push не разрешены.
Подробности: docs/performance-profile-after068.md.

## test-068 готов: перенос indexed pool references после067 (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-068.exe.
SHA256: edc7ac752545f79712685d13571efeecbe45980b1413e7dc6939be32ac7f9afc.
MSVC configure/compile/link exit0, 55.594 секунды;
существующий cache сохранён, Fresh не использовался.13 source files и108
generation files синхронизированы. CPU startup-distance-after067-v1:
4778 Original/44 SDK/5 hardware/0 pending; audio-rsp-v1 переиспользована.
Упаковка exit0;216 старых файлов вне logs сохранили size/mtime,
shared DLL подтверждены по SHA256.067 EXE/PDB и compile receipt сохранены.
ROM/settings/saves/старые EXE не переписывались.

В готовом EXE статическое чтение MSVC address instructions в8002A650
подтверждает RDRAM offset0047EC00+index*20, то есть перенесённый cache8047EC00.
Это исправляет установленный источник неверного model pointer в world draw;
также закреплены68 похожих roots. Stack resets067 и лимиты/дальность/чеаты
сохраняются. Тесты/CPU checks/игра/emulator/renderer/GPU не запускались.
Успешная компиляция и разбор инструкции не означают успешного игрового
перехода: пользователь068 пока не проверил. Commit/push не выполнялись.
Основание: research/windows-test068-binding-analysis.json и текущий
research/windows-build-check.json; предыдущие receipts в research/history/.

## Второй вылет:067, пропущенные indexed pool roots (2026-10-08)

Пользователь сообщил «ииии вылет там же». Свежий log1791451300644 проходит
handoff Native→Original, загрузку scene1 и создание геометрии. Windows1000/1001:
exception C0000005, RVA00159F29. Статический разбор EXE067/PDB связывает его
с801BC4EC, чтением model+0C в801BC5B0 после actor+74. Это следующий дефект;
переполнение стека066 на этом переходе в067 уже не повторяется по журналу.

Original8002A650 связывает component actor+74 с cache[slot]. Ранее8002A668
остался с LUI801B и читал старый801AEC00 после переноса cache в8047EC00.
Статический вывод roots пропускал SLL/SRA/SLT destination writes: stale
константа из предшествующей инструкции мешала распознать indexed base.
Исправлен разбор эффекта инструкций и просмотрены все69 дополнительных LUI:
ресурсные lookup в world/menus/minigames и actor-index reads/writes в overlays.
Все1312 прежние patches сохранены без изменения;1381 patches/674 functions,
71 hooks. Независимый static LUI/indexed-memory inventory:706 references,
0 пропусков в проверенной форме; это не доказательство игровой работы.

Изменения закреплены в exact-ROM evidence и генераторе; Generated C вручную
не редактируется. Обновляется startup generation, затем один Windows068.
Дальность, увеличенные лимиты, stack reset067 и пользовательские fixes065
сохраняются. Только статический анализ/генерация/компиляция/упаковка;
тесты/CPU checks/игра/emulator/renderer/GPU не запускались. Git после проверки.
Основание: research/windows-test067-user-crash.json,
research/distance-relocation-after067.json.

## test-067 готов: исправление переполнения стека066 (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-067.exe.
SHA256: ac1d4e20a40dfb276846a105b252020ebe38c97b979a3fc888cd4a83964ffab6.
MSVC configure/compile/link exit0, 19.516 секунды;
существующий cache сохранён,9 изменённых source files и0 generation files.
CPU startup-distance-after065-v2/audio-rsp-v1 переиспользованы: inputs не менялись.
Упаковка exit0;213 прежних файлов вне logs сохранили размер/mtime;
общие DLL подтверждены упаковщиком по SHA256. EXE/PDB066 и его compile receipt
сохранены. ROM/settings/saves/старые EXE не переписывались.

Очищаются на месте все расширенные Native enemy/common arrays, включая reset
и simulation-slice entry. Статический MSVC PDB067: native_enemy_reset32 bytes
вместо1559568; gameplay reset688 вместо182448; GameplaySliceScope32 вместо82128.
Это чтение компиляторных метаданных, без вызова функций/CPU checks или запуска
игры. Исчезновение большой stack temporary подтверждено; отсутствие вылета
и игровое поведение067 пользователь пока не проверил. Дальность/лимиты066,
Original/Native/Manual и читы065 сохраняются. Commit/push не выполнялись.
Основание: research/windows-test066-user-crash.json,
research/windows-test067-stack-analysis.json; детали: docs/draw-distance.md.

## Вылет066 при входе в катсцену: очистка Native pools (2026-10-08)

Пользователь сообщил немедленный вылет при загрузке начальной катсцены.
Его runtime.log1791450497545 заканчивается «NATIVE LOOP drained frames=224;
Original resumes». Windows Application1000/1001: exception C00000FD,
RVA00FD7D37. Статический разбор именно EXE066/PDB связывает RVA с __chkstk;
native_enemy_reset имеет stack frame1559568 bytes (0x17CC10): periods={}
создаёт временную таблицу расширенных570 optional<Period>. finish_session
вызывает её сразу после последней строки журнала через gameplay reset.
Это переполнение host stack при handoff, а не доказанное переполнение
guest actor/DL/cache buffers. Игра/эмулятор/CPU checks не запускались.

Очистка теперь поэлементная на месте, optional::reset освобождает историю
и её ресурсы. Аналогичные aggregate resets всех расширенных common arrays
в gameplay reset и GameplaySliceScope заменены тем же helper. Сохраняются
570 slots, epoch/cancellation/handoff ownership, Original/Native/Manual и
остальные изменения066. Не увеличивается стек и не отключается очистка.
После компиляции размеры frame читаются статически из нового PDB.
Игровое подтверждение остаётся за пользователем; commit/push не разрешены.

## test-066 готов: дальность рисования (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-066.exe.
SHA256: 75b9021879523cee60fc14fba423270df02719865bccffd07ebe06bdc0fa6e10.
Graphics → Object / NPC draw distance после Texture filtering:1x..5x,
сохранение выбора, общий Original/Native/Manual backend. LOD не добавлен.
570 common/1536 total actor slots с отдельными menus/geo/effects/sky ranges;
2048 sprites,1024 graphic records,4096 cache entries;65536 matrices и
524288 DL commands на банк,3MiB дополнительной игровой кучи. Near objects
имеют резерв, true allocation failure получает повтор; сроки снарядов/атак,
Source cadence и предыдущие065 cheats/reset/input сохраняются.

Generation startup-distance-after065-v2:4778 Original functions,44 SDK,
5 hardware,0 pending; instruction/hooks закреплены в генераторе и exact-ROM
evidence. MSVC configure/compile/link exit0,62.171 секунды;
37 изменённых source files и108 generation files, existing cache сохранён,
Fresh не использовался. Упаковка exit0;210 прежних файлов вне logs сохранили
size/mtime, DLL подтверждены упаковщиком по SHA256. ROM/settings/saves/старые
EXE не переписывались;065 EXE/PDB/receipt сохранены.

Тесты/CPU checks/игра/emulator/renderer/GPU не запускались. Компиляция
подтверждает сборку, а не игровую работу новой дальности. Проверка пользователем
ещё не получена; commit/push не выполнялись. Подробности: [draw-distance.md](draw-distance.md).

## Дальность рисования: подготовка test-066 (2026-10-08)

Завершена настройка Graphics1x..5x, qualified Source thresholds/projection,
1536 total/570 common actors с сохранением menu indices, расширенные gfx/cache,
65536 matrices/524288 DL commands на банк и3MiB дополнительной игровой кучи.
Far admission оставляет резерв близким; group allocation failure повторяется.
Original и Native/Manual adapters согласованы. LOD не добавлен по анализу ROM.
Тесты/игра/CPU checks/renderer/GPU не запускаются. После завершённого кода
сразу Windows build по правилу пользователя; Git после его проверки и команды.
Подробности: [draw-distance.md](draw-distance.md).

## Пользовательская проверка065 и разрешение Git (2026-10-08)

После test065 пользователь сообщил: «работает, коммит и пуш». Это подтверждение
работы читов после исправления HP и прямая команда сохранить весь текущий
проверенный этап в существующей main и отправить origin/main. Конкретные
сцены/FPS/число проверенных персонажей в065 этим сообщением не заданы.
QA записана отдельно: research/windows-test065-user-confirmation.json.
EXE064/065 и compile/package receipts сохраняют исходное происхождение.
После065 игровой код не менялся; нового билда, тестов или запуска игры/
renderer/GPU агентом не было. Для следующих изменений остаётся правило:
сразу билд после завершённого кода, commit/push после проверки и команды.

## Test065 готов: исправлен backend HP (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-065.exe.
SHA256: 8c1d6cea11c1f03b90a7c22f888038a04ae1a53e3a206b3ccced9e87b9a86127.
После пользовательского отказа064 исправлено знаковое расширение адреса
максимума HP и убран лишний availability gate. Собранный MSVC065 object
статически использует правильный RAM offset1B838B для slot0 вместо064
offset1001B838B; прочие слоты следуют Original stride1C. Это чтение compiler
disassembly, не CPU test или игровое подтверждение. Разбор: [hp-cheat-after064.md](hp-cheat-after064.md).

По действующей прямой инструкции после исправления сразу собран/упакован065.
MSVC configure/compile/link exit0, 12.75 секунд;
существующий cache сохранён. Пересобран backend и штатные build-local RT64
queue adapters, затем выполнена линковка; C-код игры не пересобирался.
Скопированы11 изменённых source files и0 generation files; прежняя CPU
startup-cheats-after063-v3 и audio-rsp-v1 переиспользованы по совпавшим inputs.
EXE/PDB064 сохранены, прежний receipt064 — research/history/.
Все207 старых файлов вне logs сохранили размер/mtime; DLL SHA256
совпадают, ROM/settings/saves не переписывались. Предметы064, settings и
reset/input/cutscene063 сохраняются. Тесты/CPU checks/игра/emulator/renderer/
GPU не запускались. Пользователь проверяет065; commit/push не выполнялись.

## HP чит064: пользовательский отказ, исправление для065 (2026-10-08)

Предметы в064 по сообщению пользователя «вроде сработали»; HP убывает при ударе
и не заполняется у всех проверенных персонажей. Найдена статически доказанная
ошибка signed/unsigned адреса maximum в backend; исправлено полное приведение
guest address и убран отсутствующий в Original availability gate. Предметы,
хуки/FPS и предыдущие fixes сохраняются. Разбор: [hp-cheat-after064.md](hp-cheat-after064.md).
Сразу готовится инкрементальный065 без тестов/игры; commit/push после проверки.

## Test064 готов: читы персонажа и предметов (2026-10-08)

По прямой инструкции «билд сразу после завершения работы» собран и упакован
Windows064 из dirty main78dc597. EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-064.exe.
SHA256: bc7b30e6ad224d80876d4840d96eee1ad734eb03d7b92f6055940c9eeb2b31e2.
Вкладка Cheats после Controls: Infinite HP по максимуму выбранного персонажа
и семь независимых 99 с заморозкой предметов; сохранение настроек и Cheats On как у Dora64.
Original/Display/Native/Manual, reset/input/cutscene исправления063 сохранены.
Подробности: [cheats.md](cheats.md).

MSVC RelWithDebInfo configure/compile/link exit0, 51.937 секунд; готовый
кэш сохранён, Fresh не использовался. CPU startup-cheats-after063-v3,
audio-rsp-v1 прежняя. Скопированы27 изменённых source files и108 generation files,
полный source ZIP не создавался; отличающиеся staging данные и EXE/PDB063 сохранены.
Все204 прежних файлов вне logs сохранили размер/mtime; общие DLL
проверены упаковщиком по SHA256. ROM/settings/saves не переписывались.
Тесты, CPU checks, игра/emulator/renderer/GPU не запускались. Успешная
компиляция не подтверждает игровую работу читов. Пользователь проверяет064;
commit/push не выполнены и разрешены только после его проверки и команды.
Receipt: research/windows-build-check.json; предыдущий063 receipt в research/history/.

## Сборка читов064 разрешена (2026-10-08)

Пользователь уточнил: после завершённых изменений сразу делать Windows build;
commit/push только после его проверки и команды. Подготавливается test-064
на актуальном dirty main78dc597; generation startup-cheats-after063-v3,
audio-rsp-v1 и готовый Windows cache. Тесты/игра/GPU не запускаются.

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

## Test062 готов: диагностика игровых обработчиков ввода (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-062.exe.
SHA256: 6b47f139f8e0f3c95b44d270c96ff94bb29ba534826236da82b7856719522a4b.
По прямой команде пользователя062 собран/упакован из dirty main09ffb36;
MSVC RelWithDebInfo configure/compile/link exit0. CPU startup-menu-reset-v1
и audio-rsp-v1 переиспользованы по совпавшим входам. Скопированы только
11 изменённых staging files; полного source ZIP нет, PDB/EXE061
и receipt061 сохранены. Все198 прежних файлов вне logs сохранили
размер/mtime; общие DLL проверены упаковщиком по хэшам. ROM/saves/settings
не менялись. Тесты и игра/emulator/renderer/GPU не запускались, commit/push нет.

controller-input.jsonl schema2 теперь видит Source dispatch/completion,
игровой/script/menu consumer, выбранные/latched buttons, counters/processes
и флаг/records пропуска заставки. Это диагностика ещё не установленной
причины отказа061; сам input loss не заявлен исправленным. Разбор:
[input-loss-after061.md](input-loss-after061.md). Пользователь проверяет062.
Новое правило пользователя закреплено в AGENTS.md: после завершённых
изменений кода сразу делать билд; Git только после пользовательской проверки.

## Действующее правило сборки: после завершённых изменений кода (2026-10-08)

Прямая инструкция пользователя: «после того как ты сделал все изменения в коде,
делай билд». После завершения очередного согласованного набора изменений кода
сразу собирать и упаковывать новый Windows EXE в D:/Games/DoraRecomp3/testbuild;
не ждать отдельной команды «билд» или «нет». Не собирать после каждого промежуточного
файла; завершить изменения текущей задачи, затем сделать один билд. Это заменяет
прежнее требование отдельного разрешения на каждую следующую сборку.
Документы/receipts после сборки не требуют нового EXE. Тесты и запуск игры агентом
по-прежнему запрещены. Commit/push только после пользовательской проверки
и соответствующего разрешения, никогда автоматически вместе с билдом.

## Test061: диагностика игрового ввода (2026-10-08)

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

## Test-060: Reset game (2026-10-08)

Windows EXE060 собран инкрементально и упакован по явной команде пользователя.
CPU generation startup-menu-reset-v1, прежняя audio-rsp-v1. Тесты и игру не
запускать; новых commit/push до пользовательского теста не делать.
Пользователь отдельно потребовал ускорить билды: staging выполнять напрямую
в Windows, копировать изменённые файлы, использовать готовый cache. Не хэшировать
повторно все старые EXE через WSL и не создавать полные архивы. Упаковщик
запрещает перезапись старого EXE и проверяет общие DLL; сохранность прочих
файлов контролируется размером/mtime. Подготовка через /mnt/d была остановлена.

## Следующая сборка: Reset game после059

Reset реализован и скомпилирован по translation units без Windows EXE.
Следующая CPU generation: build-local/startup-menu-reset-v1 (audio-rsp-v1
прежняя); использовать актуальный tools/prepare_runtime_timing.py.
В Windows staging добавить src/game_reset.cpp и src/game_reset.hpp, новый
CMake source и обновлённые src/tools. Нужна новая явная команда на билд;
номер определить по фактически существующим EXE. Подробности: menu-reset.md.

## Действующее правило Git: только после пользовательской проверки

Прямая инструкция пользователя: «коммит и пуш делай только после моей проверки
а не сразу с билдом или в конце фикса». Это отменяет прежнее автоматическое
сохранение законченного fix/build через commit/push до игровой проверки.
Исправления, документацию и сборки до подтверждения пользователя оставлять
незакоммиченными в существующей main; сохранять их и не делать reset/clean.
Готовый fix, успешная компиляция или команда «билд» сами по себе НЕ разрешают
commit/push. Только после подтверждения результата пользователем сохранять
соответствующий проверенный этап коммитом и отправлять origin/main.
Правила без тестов и без локального game/renderer/GPU запуска остаются в силе.
Не отменять ранее сохранённые коммиты без отдельной просьбы пользователя.

## Рабочая Windows-копия и лишние архивы (обсуждение059)

Основной Git остаётся в WSL; существующая MSVC/CMake сборка использует
Windows staging с Windows-путями. В него передаются текущие незакоммиченные
правки без commit/push. Это подготовка рабочего каталога компилятора.
После вопроса пользователя о переносе исходников договорённость в ответе:
далее не создавать полный ZIP архив неизменённых/уже сохранённых Git исходников.
Сохранять лишь отличающиеся незакоммиченные/чужие данные при наличии таких
расхождений, старые generation directories, receipts и прежние EXE/PDB.
Существующие исторические архивы не удалять. Перед059 полный архив уже был
создан; новых полных архивов для этой сборки больше не делалось.

# Windows build без тестов после050

Основной репозиторий: /home/vova/projects/DoraRecomp3, main.
Последний EXE059 собран из незакоммиченного snapshot поверх e97b7e6,
с общей alpha-фильтрацией спрайтов мира. CPU generation
startup-sprite-edges-after058-v1, RSP audio-rsp-v1 прежний. Полный Windows
compile/link/package exit0;189 прежних файлов вне logs сохранены.
Обновлённые RT64 sampler и CMake копируются tools/prepare_rt64_sprite_edges.py
в build-local/windows/rt64-sprite-edges; dependency checkout/pins прежние.
При staging включать новые незакоммиченные files через git ls-files --cached
--others --exclude-standard: shader helper/generator/report должны попасть
в manifest и source snapshot. Пользователь подтвердил059 и прямо разрешил
commit/push этого этапа. Следующие исправления сохранять после их проверки.
Последний Windows cache сохранён; PDB/EXE058 и staging архивированы.
Следующий EXE после нового «нет» либо прямой команды собрать, без тестов.
Ниже сохранены прежние указания после058; для будущего переиспользования
предпочитать актуальную generation after058-v1 при совпадении её inputs.

Последний EXE058 собран из96935fc: terminal poses моделей в Source/Native.
CPU generation startup-model-terminal-after057-v2, RSP audio-rsp-v1 прежний.
Для следующих C++-only сборок переиспользовать эту generation и Windows cache,
только если её входы совпадают. Старые startup-music-handoff-v1 и draft model
terminal v1 не содержат окончательную policy table/325 hooks; их receipts не
переиспользовать.058 собран по прямой команде; следующий build после нового
«нет» либо прямой команды собрать. Старые проверки не запускать.
Windows cache/manifest обновлены, PDB/EXE057 и исходники перед058 сохранены.
Для внешнего запуска package script использовать bundled PowerShell Core
из C:/Users/black/.cache/codex-runtimes/codex-primary-runtime/dependencies/native/powershell/pwsh.exe:
в legacy powershell.exe с унаследованным module path недоступен Get-FileHash.
Build logs захватывать на уровне OS subprocess, без PS stderr-to-error pipeline.
src/audio_music_handoff.cpp включён в research/runtime/CMakeLists.txt.
В новой generation только helper8023D730 получает scoped MSVC optimize off/on.

Готовый Windows staging и cache находятся здесь (это вспомогательная копия,
не основной Git и не справочное зеркало sources):
`C:/Users/black/Documents/Codex/2026-09-27/doraemon-nintendo-64-rom-windows-linux/work/windows-src`.

Python Windows:
`C:/Users/black/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`.
Visual Studio: `C:/Program Files/Microsoft Visual Studio/2022/Community`.
CRT051: `VC/Redist/MSVC/14.44.35112/x64/Microsoft.VC143.CRT` внутри Visual Studio.

1. Сверить main/status/последний EXE. Если изменился только C++ и хэши
   inputs существующих generation receipts совпадают, переиспользовать generation
   и готовый Windows cache (как в052); полная генерация не нужна. Иначе создать
   новую generation в WSL через
   tools/generate_startup.py --rom с ROM из testbuild и новым --output-dir
   под build-local.051 использует startup-audio-test051-v1 и audio-rsp-v1.
   Встроенные ROM/generation integrity guards — часть генерации; standalone
   check/test/prebuild tools не запускать.
2. Перед обновлением staging сверить его файлы со stage-manifest.json.
   Полный архив совпадающих Git исходников не создавать. Если есть чужие или
   незакоммиченные отличия, сохранить только их до обновления. Старые generation
   directories сохранять. Перенести необходимые исходники и новую generation;
   не менять build-deps pins. Записать новый manifest с source commit и хэшами.
   Compile receipt research/windows-build-check.json НЕ включать в собственные
   source_sha256 или immutable stage inputs: он создаётся после компиляции.
3. В Windows запустить tools/build_windows.ps1 из staging с -Python, новым
   -CpuDirectory и -Jobs6. Скрипт только configure/compile, без запуска игры.
   Лог051: staging/build-local/windows-test051-build.log.
4. Только после exit0 записать новый research/windows-build-check.json:
   реальный EXE SHA256, исходники, generation, completed build log hash,
   tests_executed=false, completed_cpu_checks=0. Прежний receipt сохранить
   в research/history; старые PASS не добавлять в новую валидацию.
5. tools/package_windows.ps1 -Destination D:/Games/DoraRecomp3/testbuild
   -CrtDirectory с существующим CRT -BuildId со следующим свободным номером.
   Скрипт сохраняет общие DLL, запрещает перезапись старого EXE и не копирует ROM.
   Сверить хэши прежних файлов; logs не являются immutable input.
6. Обновить документы; до игровой проверки пользователя оставлять fix/build
   незакоммиченными в main. После подтверждения результата — commit/push
   соответствующего проверенного этапа. Команда «билд» не разрешает commit/push.
   Не запускать EXE локально.

Локальные материалы051: build-local/audio-test051-build/ (stage manifest,
архив исходников050 перед staging, Windows build log, package-before.json,
package-preservation.json). Это не CPU tests и не игровое подтверждение.


## Точечный профиль078

Явно разрешён пользователем для расследования. build_windows.ps1 -GraphicsProfile
включает только DORA3_GRAPHICS_PROFILE для10 translation units и сохраняет
clean CPU generation/DIAGNOSTICS=OFF. Журнал logs/<run>/state/graphics-profile.jsonl.
Обычная следующая сборка без ключа явно выставляет GRAPHICS_PROFILE=OFF.
078 exit0,24с,246 прежних файлов сохранены; receipt research/windows-build-check.json.
