## Готовая Windows сборка

test-066: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-066.exe.
SHA256: 75b9021879523cee60fc14fba423270df02719865bccffd07ebe06bdc0fa6e10. MSVC compile/link и упаковка завершились успешно;
игровая проверка ожидается от пользователя. Commit/push не сделаны.

# Дальность рисования после065

Graphics → Object / NPC draw distance находится сразу после Texture filtering,
как в Dora64. Диапазон1x..5x, применение сразу, сохранение graphics.draw_distance
в общих port_pc_settings.json. Отсутствующее значение даёт1x; Original profile1x,
Modern5x. Отдельный LOD не добавлен: CPU distance-LOD/BranchZ моделей не найден.

## Механизмы третьей игры

Изменены подтверждённые sphere thresholds геометрии, общих акторов и теней,
opaque/translucent passes, world projection near20/far20000 и соответствующие
draw/far paths миниигр1..4. Радиусы масштабируются квадратом множителя после
unsigned→float преобразования, поэтому нет uint32 overflow. Real XYZ, collision,
AI attack ranges, скорость, Source counters и исходные сроки жизни снарядов
не масштабируются. Для обычной загрузки сохраняются пары1500/2000 и3000/4000;
Original большой класс30000 не расширяется. Дополнительные кадры Native
не повторяют менеджер создания/удаления. Увеличенный радиус загрузки раньше
создаёт обычных врагов/NPC и может раньше запустить их исходный AI.

## Память и согласованность

Original таблица1000 акторов перенесена в804B1CD0 и расширена до1536.
Общий диапазон30..229 →30..599:570 slots. Старые меню600..999 сохраняют
свои индексы; Source world geometry300..599 переносится в1000..1299,
secondary effects230..279 в1300..1349, shadow templates297/298 в1367/1368,
sky299 в1369. Не пересекаются меню, sky, geo и common pool. Minigames сохраняют
свои исходные actor indices и получают ту же relocated table. Полная Source
инициализация/очистка1536, Native arrays/range/parent validation570 обновлены.

SDK pools перенесены: sprite300→2048;100-entry graphic records→1024;
4-entry pool→64; resource cache512→4096. Все тела/инструкции переноса
привязаны к ROM/section fingerprints в research/draw-distance-rom-evidence.json;
tools/distance_patches.py включает их через поддерживаемые instruction/hooks
N64Recomp. Generated C вручную не меняется. Два совпадения80180020 в packed
resident data не являются установленными actor pointers и не изменялись.

Игровая память80400000..80800000 использует уже выделенную runtime RAM;
runtime cart handles/patch data с80800000 не занимают. Actor/cache/gfx regions
не пересекаются. После Original heap exhaustion есть отдельная3MiB spill heap
80500000..80800000 с16-byte headers/alignment, best-fit и coalescing free.
Allocator/free/reset adapters знают этот домен; DMA/resource validation,
retirement и Native pointer guards/private images расширены до8MiB.
Native bitmap sequence readers обновлены к новой resource table/capacity,
water/sand/sprite hooks предыдущих сборок сохранены.

Каждый из двух task headers и исходные view/projection/light data остаются
по Original адресам. Dynamic matrices отдельно:1000→65536 на банк;
main DL16000→524288 команд на банк (4MiB). Эти арены выделяются через runtime
allocator и живут до reset barrier. Все6 matrix producers/draw addresses,
исходный initialization prefix и task.data_ptr/size перенаправлены согласованно.
Task/message/framebuffer identities и двухбанковое владение сохранены.
Lights/retirement находят банк по реальному DL start. RT64 extended address
mode включён только на разбор task с известной ареной, затем восстановлен.
Native texture clone arena использует тот же новый DL, полный marked pointer
и прежнюю защиту копий/submit fallback.

Для дальних дополнительных spawns сохраняется резерв80 actor slots,
2MiB непрерывного свободного heap block,128 sprites и256 resources.
Близкие Original-range spawns имеют приоритет. Истинный отказ group allocator
вновь делает home record доступным; сюжетные исключения не получают retry.
Перед выдачей draw commands контролируются matrix/DL остатки; header/view и
основная гостевая RAM не служат продолжением переполненного буфера.
При исчерпании резервов дальние объекты появляются позже, после освобождения
места/приближения; полный максимальный радиус не обещается при любой нагрузке.

## Проверка и передача

Основание — текущая main a9a8d2c, ROM410f99bd… и пользовательски подтверждённый065.
Разрешены только статический анализ, необходимая генерация и компиляция/упаковка.
Тесты/CPU checks/игра/emulator/renderer/GPU не запускались. Синтаксическая
компиляция двух новых backend units завершилась без ошибок; это не игровая QA.
Следующий EXE собирается по прямому правилу пользователя после завершения кода.
Commit/push после его проверки и команды. Старые EXE/ROM/settings/saves/DLL
и compile receipts сохраняются. Границы/появление/меню/переходы/FPS проверяет
пользователь в настоящих сценах; успешная компиляция не подтверждает поведение.

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

