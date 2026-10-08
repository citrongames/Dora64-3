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

# Читы персонажа и инвентаря

2026-10-08. База main78dc597; последняя Windows-сборка test-063.
Новая задача: оформление/расположение как у первого Dora64, свои восемь пунктов.
Эталон Dora64 только прочитан: src/system_overlay.cpp и src/doraemon_cheats.cpp;
соседний проект, sources/, dependency checkouts/pins не изменены.

## Интерфейс и настройки

Escape / Back–Select открывает PC menu. Вкладка Cheats находится после Controls,
как в Dora64. Infinite HP и семь отдельных item switches; справа внизу Cheats On
при любом включённом пункте. Масштаб720p, отступ12, padding8×5, rounding5,
цвет/alpha совпадают с первым портом. Содержимое вкладки прокручивается отдельно
от вкладок и Continue / Reset game / Exit game; прежние подтверждения сохранены.
Меню уже использует латиницу, поэтому предметы подписаны читаемыми латинскими
именами без добавления/распространения сторонних японских шрифтов:

| Пункт | Название из ROM | ID | Счётчик |
| --- | --- | ---: | --- |
| Zenkai fuku no mi (99) | ぜんかいふくのみ | 1 | 801B3701 |
| Fukkatsu capsule (99) | ふっかつカプセル | 2 | 801B3702 |
| Energy stone (99) | エナジーストーン | 3 | 801B3703 |
| Gas spray (99) | ガススプレー | 4 | 801B3704 |
| Fushigi na mi (99) | ふしぎなみ | 5 | 801B3705 |
| Silver key (99) | ぎんのかぎ | 8 | 801B3708 |
| Gold key (99) | きんのかぎ | 11 | 801B370B |

По умолчанию всё выключено. Поле cheats в port_pc_settings.json, version15;
каждая опция сохраняется независимо. Переключатели переживают перезапуск
приложения и Reset game. Graphics profiles, controls и другие настройки их
не сбрасывают. UI публикует только атомарную маску; записи RAM делает game thread.
Отключение прекращает заморозку, уже выданные HP/предметы не отнимаются.
Штатное ручное сохранение игры может сохранить выданные количества; агент
существующие настройки/сейвы/ROM не переписывал.

## Подтверждение по оригинальному ROM

Japan N3DJ revision0, SHA256410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
research/cheats-rom-evidence.json хранит только адреса/хэши и названия/IDs,
без ROM, извлечённых текстур или generated game code.

801C9364 связывает выбранный byte801B374A со status801B361B+2*slot и
parameters801B8370+1C*slot. Status+0 — доступность; status+1 — HP.
Параметр+18 — signed byte maximum; шесть слотов имеют8,6,6,6,10,8 HP единиц.
Чит читает максимум из действующей таблицы Original, не назначает общий предел.
Реальное полное лечение801B56F4 использует ту же таблицу и поле; HUD801CA51C
сопоставляет те же live status/maximum pointers. Чит изменяет только HP выбранного
персонажа, не меняет availability и не открывает остальных и не заменяет здоровье миниигр/боссов.

801C0738 складывает a0 со status+1, записывает в801C0758, затем проверяет0
и максимум. Для Infinite HP v0 подменяется именно перед этой записью: даже
последний/двойной удар не публикует нулевой HP. Все прочие команды урона,
отбрасывание, анимации, падение/возврат и игровые переходы остаются у вызывающего
оригинального кода; бессмертие от потери сердец не означает отмену всех hazards.

ROM CP932 names уCE360/CE34C/CE338/CE328/CE31C/CE2F4/CE2C0 входят в pointer
table8020EF88 (ROM C6B08), сIDs1/2/3/4/5/8/11.801F09CC получает ID ячейки,
801F22D4 передаёт его getter801B5178 и индексирует имя этим же ID. Getter
читает signed byte801B3700+ID; -1 означает отсутствующий предмет. Общее изменение
801B50F4 складывает a1 и ограничивает byte0..99. Bank801B3678 является отдельным
счётчиком и не подменяется. Например штатная revival команда801F2840 лечит
выбранный в игровом меню slot и расходует ID2 через801B50F4: лечебный эффект
сохраняется, количество при включённом соответствующем чите остаётся99.

## Подключение, FPS и reset

tools/cheat_patches.py проверяет exact ROM/function/table hashes и instruction
words; tools/generate_startup.py закрепляет хуки и их declarations/inputs.
Generated C вручную не изменялся. N64Recomp сохраняет последний hook на site,
поэтому новые hooks явно объединяются с существующими trace/reset текстами.
Общий800278E0 сначала сохраняет прежний reset poll, затем применяет refill,
до Original/Native message dispatch. Character bind801C93CC также применяет
refill сразу после смены status. Между UI и guest нет общих cached pointers.

Counter-store hook подменяет записываемое значение на99 только для включённого ID;
исходные register writes, signed-byte clamp и return sequence сохраняются;
caller effects, cues, дверные/progression gates не повторяются и не пропускаются.
Getter refill охватывает чтение сразу после save/new-game restore до следующего
общего message tick. Disabled cheats проходят оригинальные инструкции.
Дополнительная частота кадров не создаёт расход/урон: Original20/30, Display и
Native/Manual используют одни и те же HP/inventory операции. Loaded common
overlay guard и bounds0..5 защищают чтение таблицы при переходах/загрузке/reset.
Прежние reset/input/cutscene fixes из060..063 сохраняются.

## Валидация и сборка

Только статический анализ ROM/исходников, необходимая генерация и компиляция.
Окончательная CPU generation startup-cheats-after063-v3:4778 функций,
44 SDK замены,5 hardware adapters,0 pending. Добавлены3 смысловых имени,
legacy ABI сохранён. Скомпилированы15 затронутых C/C++ единиц в объектные
файлы (Linux C17/C++20), все exit0; после уточнения store повторно только
port_cheats.cpp и funcs_14.c. Предыдущие records сохраняют своё происхождение,
совпадение с окончательными исходниками установлено статически по SHA256.
Receipt: research/cheats-compile.json; compiler logs: build-local/cheats-compile.
Без линковки или создания нового Windows EXE.
Тесты, CPU checks, игра/эмулятор/renderer/GPU не запускались. Визуальное меню,
смена персонажей, расход всех предметов, двери и reset требуют проверки
пользователя; успешная компиляция не подтверждает их игровое поведение.
Пользователь уточнил текущие правила: после завершения изменений сразу делать
Windows build; commit/push только после его игровой проверки и команды.
Подготавливается test-064 из актуального dirty main78dc597, с готовой CPU
generation startup-cheats-after063-v3 и прежней audio-rsp-v1. Test063 читов
не содержит. Старые EXE/ROM/settings/saves/DLL сохраняются.
