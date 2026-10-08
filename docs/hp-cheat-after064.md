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

# HP чит после064: адрес максимума

2026-10-08. Пользователь проверил064: предметы «вроде сработали», но HP
убывает при ударе и не восстанавливается до максимума у всех проверенных
персонажей. В настройках его запуска infinite_health=true. Это дефект064,
предыдущая компиляция не была доказательством игрового поведения.

## Причина и исправление

MEM_BU(offset,reg) вычисляет адрес через reg+offset, byte xor3 и вычитание
FFFFFFFF80000000. Для ручного backend нужно сохранить знаковое расширение
32-bit guest address. В064 health_offset был unsigned: сумма с int32_t base
становилась unsigned32, затем в64-bit вычислении теряла это расширение.
Для slot0 это давало RAM+1001B838B вместо настоящего RAM+1B838B.
Статически прочитаны уже собранные GCC и MSVC064 object disassembly:
GCC после сложения использовал edx без cltq; MSVC делал imul eax/ sub eax,
xor rax3/ add rax80000000 без movsxd перед чтением maximum.
Байтовая логика предметов использует offset0 и int32_t полного адреса;
в остальных обращениях port_cheats.cpp такого смешения нет.

Теперь весь unsigned guest address вычисляется до приведения к int32_t,
а MEM_BU получает literal offset0. Это тот же способ, что уже применяется
для selected character, inventory и других native adapters проекта.
Original801C9364 связывает status801B361B+2*slot и parameters801B8370+1C*slot;
Original801C0738 и HUD801CA51C читают максимум из parameter+18.
Шесть исходных максимумов8/6/6/6/10/8 сохранены. Не вводится общий предел.

Также удалён лишний availability gate: Original health change не проверяет
status+0. Выбранному slot0..5 заполняется только HP+1; progression/availability
и другие персонажи не изменяются. Проверка действующего common overlay и
границы0..5 остаются. Существующий health-store hook перед lethal check,
общий message refill и character-bind refill сохранены. Это один общий
helper для Original/Display и Native/Manual, не отдельная FPS политика.
Предметы, восемь UI switches, сохранение settings, reset/input/cutscene063
и generated code не меняются. Пользовательские ROM/settings/saves сохраняются.

## Валидация и сборка

Только статический разбор исходников/ROM/уже созданных пользовательских logs,
чтение compiler disassembly и компиляция. Тесты/CPU checks/игра/эмулятор/
renderer/GPU не запускаются. Новая генерация не требуется: хуки, каталог
и generation inputs startup-cheats-after063-v3 не изменились.
После исправления сразу собирается Windows065 на существующем cache;
коммит и push только после пользовательской проверки и команды.
Игровой результат нового backend ещё не подтверждён.
