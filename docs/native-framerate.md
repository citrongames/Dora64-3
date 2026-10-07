## Действующее правило после050: без тестирования (2026-10-07)

Прямая инструкция пользователя из нового чата отменяет все прежние требования
автоматических тестов, CPU/component/regression checks и полного pre-build,
включая проверки только изменённых частей. Не запускать прогоны персонажей,
врагов или сцен и не создавать новые тесты. Игра, эмулятор, renderer и GPU
локально не запускаются; игровую проверку выполняет пользователь.
Разрешены чтение и статический анализ исходников/ROM/дизассемблированного кода,
необходимая генерация кода, компиляция и упаковка; ошибки компиляции устранять.
При необходимости адаптировать сборочные скрипты без подделки результатов.
Исторические PASS относятся только к своим исходным входам.

Баги принимать постепенно: исправлять очередную часть и спрашивать, есть ли ещё.
Один новый Windows EXE собирать после нового ответа «нет» или сразу по прямой
команде собрать. Старое «нет» перед050 не разрешает следующую сборку.
Перед каждым запросом сверять Git, последнюю сборку и актуальные документы.
Законченные изменения сохранять коммитами в существующей main и отправлять
в origin; чужие изменения, прежние EXE/ROM/settings/saves/DLL сохранять.

## После050: исправление нарастающей задержки звука (2026-10-07)

Пользователь сообщил растущий audio lag через пару минут на всех FPS,
который затем не исчезает. Статически установлены лишний minimum368 при
потреблении367.5 кадров/update (22050/60) и потеря полной длины SDL queue
из-за cap одного DMA. Добавлены полный queue feedback, minimum на один
16-frame блок ниже и насыщение перед unsigned subtraction в ROM8004E674.
Общий resident audio path покрывает Original/Native, музыку/голоса/эффекты.
Подробности, точные адреса и ограничения: [разбор аудио](audio-queue-drift-after050.md).

Код и имена изменены; тесты/игра/генерация игрового кода/компиляция не запускались.
Исторические PASS не подтверждают правку. Последний EXE —050, новой правки
в нём нет. Пачка открыта до нового «нет» или прямой команды собрать.

## Test050: материалы и аудит камеры после049

Исправлены непрерывные циклы воды, waterfall и спирали выбора предметов.
Подключены промежуточные samples sand второго мира и связанных CI4/IA8
материалов. Camera audit4778 Original functions обнаружил зависимость обычной
камеры от player eligibility и два auxiliary Source-only пути; добавлены
независимые camera equations и правильные overlay/module identities.

Actual ROM35 sequences/339 pairs:674 Source transitions,658 scroll и1626
CI4/IA8 samples;35 paused/rebound owners. Real GIMO0A45/GIMM0A46:86 coupled
angle/scale samples. Camera/UI:192 blocked-player owners/16 ordinary modes,
220 auxiliary cases/4views, оба Source divider и60..360 FPS. Source RAM,
time/flags/context и ownership сохранены.

Пользователь закрыл пачку «нет», затем остановил оставшиеся общие проверки:
«проверки только изменённых частей эти уже не надо». Это последняя политика.
26 актуальных CPU scopes PASS, включая изменённые материалы/модель/камеру
и нужные связи;6 scopes исключены, полный suite не объявляется PASS.
Пройденный gravity log предшествует изменению inactive test-only fixture;
он сохранён, но исключён из свежего proof. Enemy был остановлен при компиляции,
scene transition/frame/story не повторялись; full-player исключён ранее.
Игра/renderer/GPU не запускались. Визуальная плавность и color/filter quality
ещё не подтверждены. Windows050 собран и сохранён; прежние файлы сохранены. Этап сохраняется в private main.

## Test-049: семь пачек исправлений после048

2026-10-07, общий camera audit после049: кроме15 ordinary roots, scripted4views,
9 minigames и2D map просмотрены все Source camera/view/SDK callers. Найдены
два Source-only auxiliary rate paths и зависимость обычной камеры от player
eligibility. Добавлены independent camera frame equations/2 auxiliary profiles,
17 Source reachability pins, overlay identity, имена и register roles. Новый код
ещё не проверен/не собран; закрытия текущей пачки не было.
Подробности: [аудит всей камеры](native-camera-audit-after049.md).


После049: пользователь сообщил оставшиеся water/spiral jumps и Source-FPS sand.
Общий material block дополнен полным cycle/Source-age continuation, CI4/IA8
deformation samples, palette-count14/16 seams, точными component/TLUT и nested
GIMO segment bindings; spiral repair теперь учитывает coupled scale reset.
Actual wiring67 room references/25 bitmap tables/35 sequences разобрано по ROM.
Новый код ещё не проверен/не собран: batch остаётся открытым до явного «нет».
Подробности и границы: [исправления материалов после049](native-regressions-after049-materials.md).


Пользователь закрыл список багов ответом «нет»2026-10-07. Исправлены стык воды/
спирали, диалоговая стрелка, white fades, все четыре области stage map и cursor
sampling, Kinopper patrol turn, Mini8 explosion contacts и body/shadow/matrix
связи, race/plane follow/finish circular camera и one-shot/result clip boundary.
Круг выбора персонажей сохраняет проверенный исходный input/phase cadence.

31 connected CPU scopes PASS; общий статус PARTIAL_USER_SKIP. Часовой player
proof исключён по прямой просьбе. Исходные commands/input/RNG/resource cadence
не повторяются в дробных кадрах. Обе исходные частоты и Manual60..360 проверены.
Исторические PASS не подменяют новую проверку. Игра/renderer/GPU локально не
запускались; визуальный результат и конкретная character art остаются проверкой
пользователя. Детали и точный охват — native-regressions-test049.md.
Windows049/package завершены; старые файлы сохранены. Commit/push main.

## Текущие пачки после048 — проверки ещё не запускались

2026-10-07: сохранены правки циклической воды/спирали, диалоговой стрелки,
белых fades, четырёх областей карты/bitmap sampling и periodic turn KinopperT1.
Добавлены Mini8 Source contactXZ publication для урона и связь дробной body/shadow
позиции с пересчётом матриц; runtime подтверждение ещё отложено.
Circular camera recurrence гонок/самолётов исправлена для follow/finish всех4
views, включая подтверждённый зимний вариант; проверки ещё отложены.
Terminal pose boundary после атак/результатов сохранён для38 exact Source end
queries, включая attack3->idle0 и все6 character result variants; тесты отложены.
Ускорение круга персонажей статически не подтвердилось; Source тайминг сохранён,
actual comparison остаётся перед билдом. Аналогичные turn contexts разобраны.
Подробности, ограничения и оставшиеся проверки —
[native-regressions-after048-batches.md](native-regressions-after048-batches.md).
Это незапущенные/непроверенные изменения поверх048, не новый PASS или EXE.
Новые тестовые фикстуры пачки4 отложены после отказа автоматической проверки
разрешений. Тесты, генерация, один Windows build и commit/push main — после
явного «нет» пользователя. Часовой полный player proof остаётся исключён.

## Test-048: причины регрессий карты, камер и моделей

Исправлены обратный countdown/easing stage map и единая дробная прокрутка
фона/значков, нарезка cursor bitmap, два временных +180 planes pitch,
классификация camera/global loads при реальном game SP801AEAF8, paused/zero-rate
позы и Source частота byte oscillator боковых item arrows. UI-only policy
выбирает текущий frame timestamp для фейда. Проверены аналогичные общие paths.

31 connected CPU scope PASS; общий набор PARTIAL_USER_SKIP, full-player
исключён по прямой просьбе.414 UI cases,332 Original endpoints,8682 samples;
187 minigame cases/1425 slices при фактическом SP, полный bomb update;
actual paused/zero-rate SDK pose/morph/override cases.291 scene и217 enemy
групп сохраняют исходные completed execution logs и точную эквивалентность
неисполняемой render ветви; один текущий полный вариант каждой области свежий.
Полная связь конкретного voice/fruit симптома с paused pose ещё визуально
не подтверждена. Игру/renderer/GPU локально не запускали. Подробности —
native-regressions-test048.md. Windows048/package завершены; старые файлы сохранены. Commit/push main.

## Test-047: исправления после046 и аудит связанных путей

Windows047 из startup-regressions-test047-v1 сохранён в общей testbuild папке.
31 завершённая connected CPU проверка PASS; full-player USER_SKIP по просьбе
пользователя, общий набор PARTIAL_USER_SKIP. Игра/renderer/GPU не запускались.
Старые completed scene/enemy cases сохраняют исходные logs и точную эквивалентность
неактивной texture ветви, свежие области и active texture проверены отдельно.

Native texture adapter плавно продолжает точные authored byte format2/CI4 scroll sequences
через RDP10.2 tile origin, generated material prefix и nested display lists.
Найдены11 семейств /94 кадра среди1475 BMP из трёх ROM таблиц;10 семейств имеют
static ID owners, для последнего owner не доказан. Это анализ данных, а не
утверждение о runtime reachability всех кадров. Палитры/bitmap/Source counters
не меняются; cloned lists принадлежат текущему frame bank до его retirement.
Проверены смена ресурса, clamp/loop/stopped, graph state и fallback при нехватке
tail. Другие форматы/различающийся art не получают выдуманных промежуточных кадров.

UI/fade eligibility охватывает стабильные world menus и активные переходы
до loader boundary. Модели выбора получают real SDK local matrices; stage-map
cursor rectangle сохраняет fractional координаты после integer Source math.
Charge HUD получает дробный RDP right edge, не изменяя charge count/cues/input.
Общий Source completion clock связывает видимые модели, меню, миниигры и texture.
Дополнительно исправлены4 viewport origins mini1/2, Mini2 visual/follower copies
после terrain query, Mini8 shadow getter copies, mini3 terminal retreat и
temporal +/-25 wall-camera distance caps. Source effect births/reset/snap,
commands/score/RNG/input сохраняют отдельные исходные решения.

Joint matrices остаются cache последней отрисованной позы для DC450 collider;
их откат к pre-session данным отвергнут после анализа consumers. Автоматическая
проверка отклонила широкую замену файла из Git; точечное удаление добавленных
matrix lease hunks сохранило остальные изменения и прошло проверку.

Визуальный результат сообщений ещё не подтверждён. Соответствие voice/fruit
конкретному модулю и actual wall-camera cause остаются непроверенными; точные
terminal/cap дефекты исправлены. Четыре auxiliary control pairs и настоящие
loading/graphics-off/framebuffer зависимости сохраняют Original handoff.
Подробности и ограничения — native-regressions-test046-worklist.md.
Понятные Source имена/адресные соответствия обновлены. Donor и32 dependency
pins неизменны; прежние EXE/ROM/settings/saves/DLL сохранены. Commit/push main.

## Test-046: блок6 интеграции завершён

Windows046 из startup-integration-block6-v1 сохранён в общей testbuild папке.
30 завершённых CPU checks PASS, включая49 сценариев реального общего цикла.
Долгий full-player прерван по прямой команде пользователя; текущего PASS
полного персонажа/NPC matrix нет. Общий pre-build завершён частично.
Native часы запускаются после завершения старого host read;
сессия закреплена за callbacks/ROM/divider/verified reload generation.
Один timestamp связывает движение, камеры, модельные позы, UI и fade.
При выходе сохраняются дробная фаза моделей и исходные дискретные Source
решения/cues без повторного исполнения команд. Удержание двух банков,
stale wakes, смена частоты/модуля, pause/seek/rebind/reset проверены CPU.
Резервные player states19/23/26/30 больше не прерывают Native мир.
Pak/file UI получил Native easing курсора/панелей и hint offset;
device/FSM/I/O и authored blink/sprite остаются Source once.

Имена и ROM-backed описания закреплены в каталоге и документах; старые ABI
и canonical C сохранены. Exact-input proofs сохраняют исходное execution
provenance. Игра/renderer/GPU локально не запускались; синтетические CPU
inputs не заменяют полное прохождение или визуальную проверку.
Четыре auxiliary startup/resource/model control пары и unsafe/unknown/
loading/framebuffer границы оставлены на явном Original fallback;
их Native движение не заявляется. Подробности — native-framerate.md.
Прежние EXE, ROM, settings, saves и DLL сохранены; donor и32 dependency
pins неизменны. Результат сохраняется commit/push в private main.

## Test-045: весь блок4 — миниигры и специальные режимы

Завершены девять миниигр и три общих режима: выбор, участники/настройка,
результаты. Windows045 из startup-minigame-block4-v1 сохранён в
D:/Games/DoraRecomp3/testbuild после30 connected CPU checks.
Original20/30 сохраняет input/AI/FSM, счёт/раунды/RNG/waits, sound,
clip/resource/spawn/free. Manual60..360 обновляет реальные уравнения
движения, повороты/масштаб, камеры, модели/эффекты и2D с исходной скоростью.

Движение участников1/4 учитывает промежуточные snapshot/projection fields.
Все три косвенных обработчика8 включены. Terrain query использует настоящую
CPU геометрию и возвращает scratch/stack. Контактные импульсы остаются
однократными. Native draw сохраняет входы actor transforms, производные
матрицы, world echoes и SDK scale globals; следующие Source updates видят
свои значения. Связанные velocity/acceleration и camera moving-target rates
интегрируются по времени; Source end step и дробные partitions согласованы.

CPU доказательства:695 frozen044/current Original instruction streams,
все9 модулей/3 common callback pairs,138 Source/Native cases и946 slices,
20/30Hz и Manual60/73/120/144/165/360, command-once и Source restoration.
Connected suite проверяет прежние объекты/NPC/врагов/оружие/урон/предметы,
камеры/катсцены/UI/модели/эффекты/ввод, переходы, очереди и кадровые банки.
Воспроизводимые cache entries сохраняют исходное execution provenance;
изменённые зависимости запрещают reuse.

2593 описания,2262 смысловых имени функций и5774 scoped variable names
закреплены через ROM-qualified каталог; ABI и canonical C сохранены,
адресные соответствия и документы обновлены. Работа сохраняется в private
main коммитом/push; архивы только дополнительные. Прежние EXE, ROM,
настройки, сохранения и DLL сохранены. Donor и32 dependency pins неизменны.

Первая Windows compile попытка в ограниченной среде завершилась C1902.
Версии компонентов MSVC совпадали; ошибка повторилась на отдельном C файле,
а та же компиляция вне ограниченной среды прошла. Windows045 собран повторно
из того же проверенного кода/генерации; оба журнала сохранены с SHA256.

CPU states/resources и достигнутые ветви синтетические; это не полное
прохождение миниигр и не визуальная проверка. Игра/renderer/GPU локально
не запускались. Общий variable FPS ещё требует блока6 — итоговой интеграции,
переходов, fallback/reset и связей всей игры. Его выбирает пользователь;
автоматически новый блок не начинается.

## Test-044: весь выбранный блок1 — объекты, платформы и оставшиеся NPC

Сборка044 из generation startup-scene-block1-v4 сохранена в
D:/Games/DoraRecomp3/testbuild после29 connected CPU checks.
Реализованы289 типов,292 точных ROM-варианта:42 NPC,5 общих объектов,
129 тестовых/EV/исчезающих/damage/platform объектов и113 stage objects.
Движение, доказанные повороты/наклоны/масштаб/прозрачность и время моделей
получают Manual dt. Wait/RNG/contact/quest/clip/spawn/retirement сохраняют
Source порядок. Original20/30 и канонический ABI/ROM identity сохранены.

Связанные траектории133/527/528/564 учитывают Source порядок родителя,
epoch и lifetime. Door109 прибавляет своё перемещение к действительным
координатам игрока, сохраняя его собственное движение. Init/reset/floor и
terminal stores отменяют устаревшие математические предложения.
Исправлено знаковое вычисление адреса Fly44–46 в Native NPC повороте.
Door109 сохраняет Native обновление common-world во время своего реального
B8ED0 затемнения(D494=16), с проверкой lifetime/epoch. Игрок C8B28 остаётся
остановленным по Source правилу BB080. Устранён повторный Source шаг при
раннем возврате из Native посреди периода; RAM/команды/модели сравниваются
точно. Дополнительный CPU сценарий принудительно сохраняет прежнее
разрешение движения игрока и подтверждает, что при fade он не обновляется.

Строгий Source replay использует1163 exact frozen/current Source bodies,
Original20/30 и Native60/73/120/144/165/360, три сценария, реальные
constructors/parser/contact/SDK/retirement,96 периодов либо настоящий
handoff сцены. Проверены связи с218 врагами, игроком, оружием, уроном,
предметами, переходами, камерами, катсценами и игровым UI.
Сохранённые полные варианты имеют checksum бинарников/логов и доказанные
неизменённые ветви; cached player groups сохраняют фактическую execution
provenance. Они не выдаются за свежие запуски.

Каталог2532 описаний функций,2201 смысловых aliases и5765 scoped variable
names проверен: canonical C, preprocessing tokens, syntax и ABI сохранены.
Документы и старые адресные соответствия обновлены.32 dependency pins и
donor неизменны; прежние EXE, ROM, настройки, сейвы и DLL сохранены.

CPU модели/клипы/геометрия/ресурсы/звук синтетические; часть EV programs
синтетическая с настоящим parser. Достигнутые состояния перечислены.
Это не доказательство всех сюжетных ветвей и настоящих уровней. Контакт
игрока с движущейся платформой и внешний вид проверяет пользователь.
Игра/renderer/GPU локально не запускались. Замечания по визуальному
результату врагов043, включая идентификацию механического кролика,
не объявляются закрытыми без пользовательской проверки.

Выбранный блок1 реализован, CPU проверен и включён в Windows044.
Весь variable FPS ещё не завершён. Следующий блок выбирает пользователь:
4 — миниигры/специальные режимы;6 — итоговая интеграция всей игры.
Блоки2/3/5 реализованы ранее; новый блок автоматически не начинается.

## Test-043: весь выбранный блок2 включён в Native

Сборка043 изgeneration362 сохранена в D:/Games/DoraRecomp3/testbuild после
полного connected CPU набора28 checks. Включены все218 ROM-qualified типов:
135 обычных,79 типов всех семи боссов, разрушенный робот478 и общие эффекты
257–259. Реализованы движение и время моделей, снаряды/части/связанные
траектории, сохранившие Source порядок AI, wait/RNG/contact/clip/birth/death.
Supaizan, Batti и Oppi входят в новую сборку вместе со всеми их вариантами.

Строгая проверка1168 Source bodies и весь218-kind replay завершены:
совокупно2616 frozen/current Original и7848 Native сценариев на60/73/120/144/165/360FPS,
Original20/30, настоящие constructors/contact/retirement и Native births.
Исправлены общий Source checkpoint времени моделей для дискретного AI,
лишние события вместо Source равенства, повторное продвижение нового
клипа после post-step seek/rebind, повторное выполнение команд родителя
после поздних дочерних записей в Source pool, pose-only вход Boss1Bironn
и лишнее следование Boss2ArmFlash в состоянии10.
Исправлено аналогичное завершение linked pose Boss6ArmFlash459.
Fannel467 начинает orbit attachment после реального state4 command,
сохраняя Source aim в переходном периоде1→4.
Enemy births в уже пройденных слотах не получают ранний дробный update;
первый model/AI/motion step начинается со следующего Source pool периода.
Исправлены точный ROM coefficient и operand f2 общего эффекта258;
test skeletons Majin/Boss3/Boss4 включают все индексированные кости.
Pyonger2 проходит монотонное приземление к Y=0 и удержание на земле между
кадрами. CPU assets/geometry/clip/sound остаются синтетическими; достигнутые
состояния перечислены в отчёте. Это не доказательство всех ветвей AI или
визуального результата в настоящем лесу фей. Соответствие пользовательского
механического кролика конкретному kind и устранение дрожания в уровне ещё
не подтверждены. Локальная игра/renderer/GPU не запускались.
Soundorago в тестовом завершении проходит собственный Source обработчик
смерти с освобождением буфера истории; эта проверка освобождения ресурсов
не выдаётся за проверку Native анимации смерти.

BroboDC08 RealT1/Hob получают доказанную lui-базу при pure pose entry;
Rikurage light использует настоящий Source lui/lw вместо unsigned offset.
Повторены32 затронутых/незавершённых типа,186 полностью пройденных
неизменённых типов сохранены с фактической provenance361. Generated diff
строго ограничен тремя Native pose branches; независимые checks сохраняют
реальные старые отчёты. Плотные totals относятся только к свежему replay.

Каталог2072 описаний функций/1741 смысловых aliases/5305 scoped variable
names проверен генератором362: исходные ABI/ROM identity и канонический C
сохранены. Обновлены family docs и адресные соответствия.32 dependency pins
и donor неизменны; прежние EXE, ROM, настройки, сейвы и shared DLL сохранены.

Реализация и предсборочная CPU проверка выбранного блока2 завершены.
В целом variable FPS ещё не завершён. Следующий блок выбирает пользователь:
1 — объекты/платформы;4 — миниигры/специальные режимы;6 — итоговая интеграция
всей игры. Блоки3 (катсцены/камеры) и5 (игровой UI) завершены ранее в041.

## Test-042 передан: первая связанная группа блока2

Сборка042 изgeneration358 сохранена в D:/Games/DoraRecomp3/testbuild после
полного connected CPU набора28 checks.25 типов/24 callback roots прошли
300 frozen/current Original и900 Native schedules; полный player/world suite
завершён на24 настройках. Добавлены152 смысловых имени функций и238 scoped
variable names; документы обновлены. Игра/renderer/GPU локально не запускались.
Все предыдущие файлы, ROM/settings/saves/shared DLL сохранены;32 pins/donor
неизменны. Следующими внутри блока2 остаются остальные семейства, связанные
части/генераторы и боссы; весь блок2 и весь variable FPS ещё не завершены.

# Блок2: враги и активный AI — этап generation358

Пользователь подтвердил исправления test-041 и выбрал блок2. Текущая группа
из25 типов прошла отдельную подключённую CPU-проверку; общий pre-build связей
подсистем завершён. EXE042 собран и передан после28 connected CPU checks. Original таблица содержит
592 типа, общий pool —200 actor slots с индексами30..229. Scene guards теперь
проверяют kind0..591; диапазон actor slots сохранён.

В коде подключены25 типов и24 ROM-qualified callback roots:

| Семейство | Kinds | ROM overlay | Разобранные состояния |
| --- | --- | --- | --- |
| KinopperT1/T2 |264/265|12FD90 .demo_slot1|1/2/10, движение/атака/смерть|
| Pyonger1/2, PyongerTama |266..268|0CEC60 .demo_slot0|1/2/3/10, связанные создание/attachment/flight/removal|
| BroboA100/A101/A103/A104, два снаряда, эффект |341..347|0CEC60 .demo_slot0|hover/patrol/return/oscillate/aim/fire/hit spin/death/projectiles/effect|
| Sabotti1/2/3 |269..271|0D9910 .remaining_slot0_04|1/2/10, pitch/orbit/dive/return/spin/scale collapse/death|
| MiniCar1/2 и связанный дым |359..361|0EDDB0 .level1_slot0|route/chase/turn/stop/resume/death, parent attachment/unlink/fade|
| PatPot и снаряд |362/363|0EDDB0 .level1_slot0|chase/roam/aim/fire/death, XYZ flight/collision/retirement|
| Powan1..4 и эффект превращения |387..391|0EDDB0 .level1_slot0|idle/dialogue/orbit/rise/exit/return, story interaction/transform/present/wave|

`native_enemy.cpp` запускает полный реальный private FSM один раз за actor-local
Source период. Wait counters/RNG/clip/sound/spawn/damage/retirement сохраняют
исходный порядок. На каждом Native интервале продолжаются реальные XYZ equations,
scalar speed, heading/roll и model time. Сохраняются реальные Original proposal
bits в конце периода; дополнительные кадры не повторяют команды AI.
Identity включает kind, даже если два типа разделяют один callback/contact.
Смена типа/состояния/lifetime начинает полный Source период от timestamp команды;
старые clip checkpoints не доставляются новому типу. Внешний motion-only write
перестраивает движение без повторения AI counters/RNG.

Общие уравнения используют разобранные функции
`dora3_object_project_heading_velocity_xz_801E7F54`,
`dora3_object_project_pitch_velocity_y_801E7FC4`,
`dora3_object_project_heading_pitch_velocity_xyz_801E801C`,
`dora3_actor_apply_gravity_probe_vertical_collision_801E810C` и
`dora3_object_step_heading_toward_player_801E88C4`.
E7FC4 — проекция скорости по pitch, не gravity. Только реальный посещённый
E88C4 или явно разобранный математический store задаёт постепенный поворот;
initialization, wall reflections и aiming offsets сохраняют мгновенные setters.
Speed ramp, physical yaw/pitch и XZ/XYZ projection связаны в одном дробном
kick-before-drift уравнении. Суммы sin/cos для меняющихся углов вычисляются по
реальной Source recurrence; момент и float order каждой Source projection
сохраняются. Отдельная slowdown после projection не меняет предыдущую скорость.
Source gravity cap-48 нормализует внешнюю скорость ниже предела мгновенно.

Brobo hover сохраняет исходные sinus phase34+10, ROM radians и base height;
переходная высота продолжает фактическое exponential approach. Реальные
acceleration/deceleration stores связаны с X/Z kick/drift. Sabotti использует
sinus pitchC+10 и Source-equivalent fractional sum для вертикального движения;
смертельный roll±12 и зависимый от private counter visual yaw продолжаются между
исходными командами. Исходные условия clamps/переходов не переписываются.
BroboA103/A104 сохраняют actual ROM jump tables, contact differences, floor/ledge
probes, матричный muzzle offset-24/130/120 и разные условия выстрела18/27.
A104/state2 (Source scalar speed0) сначала меняет turn rate5C на signed5, затем
прибавляет её к total58/yaw4; Native продолжает эту angular recurrence вместе.
Hit spin5, наоборот, прибавляет текущую integer rate38 и только после этого
уменьшает её на4. Намеренные death Y58±10/11-decision shakes остаются Source poses.

Прикреплённый PyongerTama повторно выполняет только разобранный участок Original
matrix/attachment geometry: actor yaw, XYZ translation и fixed offsets0/96/205,
без чтения bone transforms модели. Constructor, counters, animation/shot/collision
commands обходятся; актуальная parent pose сохраняет и XYZ, и heading. Родитель
идентифицируется physics7C и lifetime. Pyonger2/physics80, наоборот, хранит индекс
созданного снаряда; чтение его model time<=0 относится к связанному выстрелу.
Link7C/80 входят в command guard. Unlink7C=-1 запускает настоящую flight command,
а pose-only path никогда не переходит в ветку constructor/clip bind при unlink.
Во время дискретного callback ребёнок читает сохранённый Source endpoint более
раннего родителя; затем Native visible pose восстанавливается. Размер очередного
дробного кадра не меняет входы исходной команды. При полном Original периоде
это совпадает с реальным порядком общего pool.

MiniCar smoke использует собственный pose-only участок: копирует parent XYZ и
visual yaw, без model bones и без изменения physical yaw. Route pointer48,
waypoint42, stop marker3E и smoke link80 являются командами Source FSM.
Намеренные shake poses Brobo/MiniCar на damage/death остаются Source commands;
их private counters не ускоряются дополнительными кадрами.

Powan — сюжетные персонажи, а не обычные противники. Contact mode4 и разрешённые
states, dialogue B922C, story3748/interaction3760, thresholds4/6/8, transformation
ticks35/55 и реальные constructors/destructors сохраняются. Native equations
продолжают actual orbit200/phase42+2, rise phase36+5, vertical arc и yaw wave34+20.
Reverse animation cue0 и rise cue30 используют реальные checkpoint crossings.
Эффект391 продолжает sine scale/opacity; Source Z scale копирует X equation,
включая внешнюю anisotropic scale. Смертельный scale collapse Sabotti3 сохраняет
тот же порядок X-to-Z assignment.

Opacity продолжается только при удержанном разобранном store counter58 и точном
совпадении actual published model alpha. Native воспроизводит actual 3B168
model74/78 и E89CC physics2A flags. Powan публикует после обновления counter,
smoke — до decrement; первый bind остаётся командой. Внешняя alpha/flag запись
снимает продолжение. Неразобранная прозрачность не получает Native equation.

ModelEventCheckpoint накапливает реальные clip advances/end pulses между
решениями AI. Brobo cue18, MiniCar cue36, Powan cue30/reverse0 и Pyonger linked zero cue не зависят только от последнего
дробного кадра; смена clip run/lifetime защищает от устаревших событий.
Внешняя запись pose/velocity/acceleration меняет опорное движение и не повторяет
counter/RNG commands. Настоящая смена state/latch/counter получает новый Source
период от точного command timestamp. BlinkD8 уменьшается на локальной Source
границе; C9 pause обрабатывает общий pool/object clock.

Добавлены152 смысловых имени функций и238 scoped register names с ROM instructions,
включая actual constructors/contacts. Projectile343/344 retirement —state2,
а PyongerTama —state10; contact mode0/1/3 и blink30 закреплены отдельно.
Каталог теперь содержит1001 descriptions,670 semantic aliases и834 register scopes.
ABI symbols, tables/function pointers, ctx storage и reference sources сохранены;
документация использует aliases с прежними адресами в таблице соответствий.

Подключённая CPU-проверка generation358 сравнила300 расписаний frozen/current
Original и900 Native расписаний для25 типов в4 ROM overlays. Проверены72 000
состояний общего пула,638 850 интервалов и столько же zero-dt calls,768 настоящих
контактов. Частоты Native:60/73/120/144/165/360; исходные периоды20/30Hz.
Проверка включает фактические constructors/destructors, связанные дочерние
объекты, model clocks, контакт1/4, pause/resume, внешнюю команду pose и смену
343/344.159 целых Source функций совпали с frozen356 после удаления строго
учтённых approved timing hooks; команды AI не заменены моделями проверки.

Для Source20 сравнивается каждая исходная граница; для Source30 — каждая третья
(100ms), чтобы округление1/3ns не вошло в следующий период. Контексты вызывающей
стороны и zero-dt state проверены на дополнительных кадрах; поля не маскируются.
После принудительного завершения все реальные model/resource allocations
возвращаются к исходному состоянию. Отчёт: research/native-enemy-check.json.

Ресурсы моделей, геометрия комнаты, звук и lookup используют синтетические
данные. Не разобранные scripts/effects возвращают реальный отказ allocation,
с записью команды; весь набор death particles и все ветки каждого FSM этим
прогоном не доказаны. Реальные уровни, renderer/GPU и визуальная плавность
локально не проверялись. Все25 типов и весь блок2 не объявляются полностью
проверенными в игре.

Эта проверка выявила две реальные ошибки переноса. Общий prelude E8D44 сбрасывал
physical2A между дробными кадрами; Native теперь сохраняет фактический результат
callback/E89CC до следующей исходной команды. Это предотвращает потерю
transparency flag у снарядов и ложную отмену opacity equations. В Powan2 orbit
беззнаковый dynamic offset менял знак guest address в MEM_H и мог читать host
память со смещением4GiB; offset теперь явно знаковый. Обе исправленные области
входят в повторный успешный прогон текущей группы.

Остаются остальные ground/flight AI семейства, attached parts/generators и
боссы. Общий connected CPU pre-build для этого крупного этапа выполняется один
раз; после обнаруженных ошибок повторяются только их проверки и зависимости.
Игра/renderer/GPU локально не запускаются. К другому блоку не переходить без выбора.

---

История предыдущих этапов:

# Test-041 — исправления поворотов, меню и камеры

Сборка `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-041.exe` передана изgeneration356.
SHA256 `e7716cc810d609635f266542e63aff34e537c8d37258ae7a7a5a755cb9ad2ec8`. Исправлены дробные повороты/наклоны моделей и сценарной камеры:
Original wrapped velocity358 теперь интерпретируется как−2 между целыми шагами.
Игровое меню files сохраняет исходную скорость prompt/arrow/background/highlight
и nested rows. Common game UI входит в world policy; standalone UI ownership
только для меню, поэтому неподключённые world branches сохраняют прежний cadence.

Перед этим одним билдом завершены26 текущих CPU checks с проверкой связей подсистем:
камера/катсцены/UI,
модели/input/clocks/effects/gravity, loop/session/presentation/banks/FIFO/context,
live/newborn projectile continuation и actual first room resource barrier.
Cutscene154 cases/506 Source endpoints/1431 Native intervals; UI116 cases/234
repeated Original draw comparisons. Names96 C units,849 descriptions/518aliases/
596 variable scopes; Original ABI/register storage сохранены. Подробные результаты
и границы: `research/native-prebuild-check.json`; Windows build совпадает с
compiled source/generation.32 dependency pins и donor неизменны.

Полный player/world/weapon/damage/death/recovery тест прерван по прямой просьбе
пользователя «скипай их, давай билд». Он не объявляется PASS. Новые тесты перед
041 не запускались; оставшиеся процессы остановлены. Это исключение для041.

CPU fixtures используют synthetic assets и явные эффекты/ресурсы/graphics boundaries.
Connected pickup batch189 lifetimes на одном setting; старые exhaustive private
cross products сохраняют прежнее происхождение, не объявляются новым запуском.
Игра/renderer/GPU локально не запускались. Реальный вид первой катсцены Нобиты
на стуле, игровые меню и камера ещё требуют пользовательской визуальной оценки.
Source355/test040 архивирован; все предыдущие EXE/DLL/ROM/settings/saves сохранены.
Полный variable FPS всей игры ещё не завершён. Следующий блок выбирает пользователь:
1 объекты/платформы;2 враги/активный AI;4 миниигры/специальные режимы;6 общая интеграция.

## Исправления039/040 — этап356 / test-041

Исправлены три связанные регрессии: дробные повороты/наклоны участников и
сценарной камеры, ускорение декораций/стрелок/мигания игрового files-menu и
UI-only захват Native сессии при неподключённом мире, ухудшавший плавность камеры.
Original хранит угловую скорость−2 как358; Native теперь интегрирует её signed
представление между исходными шагами. Полный шаг, Original20/30 и delay-slot
store остаются точными. Исправлены reviewed XYZ actor rotation801E3358 и
camera angle801DFBA4. Actual files root801BFC44 защищает всё состояние6700..68C8,
включая prompt/background/arrow/highlight; Original multiple-view multiplicity
сохраняется. Common game UI входит в eligible gameplay/cutscene policy, а
самостоятельная UI policy относится только к меню. Неактивный screen rectangle
больше не требует копирования4MiB на каждом кадре.

Перед единственным Windows041 завершены26 текущих CPU pre-build checks: input,
math/model/camera/UI/effects, общий loop/banks/FIFO/context/pause, live/newborn
projectile scene-frame и actual first room resource barrier. Cutscene356:
154 cases/506 exact Source endpoints/1431 Native intervals/18 guards; UI356:
116 cases/64 motion endpoints/153 math samples/234 repeated draw comparisons/37
guards. Отдельные signed rotation cases включают144FPS. Names356:96 C units,
849 descriptions/518 semantic aliases/596 scoped variables, canonical
round trip/preprocessed C tokens/ABI/register storage. Каталог/документы
сохраняют соответствия прежним функциям. Итог research/native-prebuild-check.json.

Полный player/world/weapon/damage/death/recovery тест не завершился. Пользователь
прямо потребовал «скипай их, давай билд»; оставшиеся четыре CPU группы остановлены.
Их незавершённые результаты не объявлены PASS. Для этой сборки полный pre-build
прерван по просьбе пользователя; новые тесты перед041 не запускались.

Первый общий прогон выявил устаревшие link fixtures после новых UI/cutscene/
flight/projectile hooks и прежнюю ошибку списка Python в session checker.
Исправлены обвязки; UI/effects/process/loop/player/transition/pickup и отдельные
leaf checks повторялись только после своих ошибок. Прошедшие независимые
проверки заново не запускались. Shared runtime проверяется настоящими модулями;
off-scope script/menu roots имеют fatal sentinels. Старые gravity/motion leaf
oracles не приобретают GameplayPreview: их inactive hooks явно обозначены,
активные projectile owners покрывает завершённый scene-frame scope. Full-player
scope остаётся незавершённым.

Сохранён tools/check_native_prebuild.py для следующих крупных этапов: свежий
полный набор перед build; --only повторяет failed/affected checks. Исторические
exhaustive pickup cross products не выдаются за новый запуск: новый connected
batch включает189 полных связанных lifetimes/48384 Original endpoints на одном
setting. CPU tests используют synthetic assets и явные sound/resource/graphics
границы; реальные уровни, вид первой катсцены, меню и плавность камеры визуально
проверяет пользователь. Игра/renderer/GPU локально не запускались. Полный
variable FPS всей игры ещё не завершён; следующими остаются блоки1/2/4/6.

Самый большой full-player тест разбит на четыре независимые группы camera12/2
и Original20/30. Каждая исполняет все60/73/120/144/165/360FPS и сохраняет свои
исходные60FPS baselines. Game fixture не изменена: диагностическая копия
добавляет только selector группы и assert шести настроек. Все прежние Source
state/resource/command oracles, final counters и ASan/UBSan/LSan остаются.
Новый экспериментальный tools/check_native_player_update_partitions.py рассчитан
на четыре PASS и24 настройки; полный запуск этого launcher не завершён и его
итоговая агрегация не подтверждена. Незавершённые serial и parallel прогоны
сохранены как прерванные; они не объявляются полным PASS. Остальные
26 checks не повторялись. Два пути на исходную fixture в подготовке launcher
различены: read остаётся Original, только link использует derived copy.

## Предыдущий этап — main355 / test-040

Завершён выбранный блок5: общий2D/UI и основные title/files/intro/selection меню.
Native кадры пересчитывают panel/rectangle bounds, title motion/scale, intro alpha,
selection fixed actor/text movement. Text reveal/lifetime, blinking, pages/input,
sound requests и saves сохраняют исходную скорость и порядок. При непереведённой
мировой подсистеме UI fallback оставляет Original20/30Hz мира.

Windows: `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-040.exe`. Целевые UI проверки,
проверка имён/документов и Windows compile/package прошли; игру/GPU не запускали.
Каталог849 descriptions/518 aliases/596 scopes; новый блок39 UI функций/9 scopes.
Полный variable FPS всей игры не завершён. Следующий блок выбирает пользователь:
1 объекты/платформы;2 враги/активный AI;4 миниигры/специальные режимы;6 общая интеграция.
Другой блок не начат. Подробности: docs/windows-test-build.md,
research/native-ui-check.json, research/native-ui-connected-stage355.json.

---

## История реализации и проверок

## Продолжение main234 — проверены Source-награды и имена, runtime прежний


## Предыдущие этапы pickup/runtime

Предметы118/123/127 после настоящего Source входа обновляют no-parent
оседание2, появление4, подъём5/уменьшение6, вращение7, двухфазную дугу8
и сбор10 на Native интервалах. Actual query перед ветвью collected fade
сохраняет Original новую velocity, включая положительный выход ниже поверхности.
Original20/30/init/целые счётчики/переходы/контакты/команды/удаление сохранены.

Fresh291 basic432/110592 иsettle guards3456/884736 whole endpoints,
related reveal/arc guards/combined7/8 lifetimes, full O2 player/world/NPC3192 B,
loop иscene resource barrier прошли. Восемь291 executed origins точны;
Свежая names348:778/447/562,96 canonical C/tokens/C17/ABI; runtime main295 прежний. Negative289 opacity
ошибка сохранена явно;291 меняет query-before-fade, не ослабляет assertions.

Разлёт3 иследование за родителем пока отдельные кандидаты. Negative303
сохраняет отсутствие Native idle после VX/VZ=-0; исправленный312 short20
прошёл, full проверки предстоят. Native attached310 basic864/221184 с
36432 frozen current-parent matrices прошёл; guards/whole integration остаются.
Parent-follow/moving idle/прочие item families/прочие levels/enemies/platforms/
script cameras/cutscenes/minigames/2D/интеграция остаются. Полный variable FPS
не завершён. Test037 прежний; нового Windows EXE или запуска game/GPU нет.


## Рабочая интеграция352 — общий Native блок предметов и эффектов

В основной рабочий код перенесены разлёт3, parent-follow2, движение idle1,
Native paths стандартных111..128 и common animation129. Source сохраняет
creation/quest/contact/init/count/RNG/clip/seek/teleports/retirement. Внешние
команды и поздняя пауза возвращают неизменённые непрерывные поля к Source baseline;
новые реальные команды сохраняются. Pending opacity marker2A у129 запрещает
преждевременный Native common update. Функция владения129 получила отдельное имя.

Каталог778/447/569: дополнены семь точных ROM-qualified переменных opacity,
private callback и blink общего pool updater. Alias/ABI/register storage прежние.
Документы используют смысловые имена, generated game code/ROM вGit не входят.

Изолированные реальные proofs312/321smoke/325/328/333/338short/339/340/344/346
сохранены с точным происхождением в research/native-pickup-connected-stage352.json.
Они не объявлены полным проверенным запуском нового runtime. На340 все18семейств
прошли3888 paired worlds/995328 exact endpoints. Чрезмерные старые full312/321,
guard321/332 иfull338 scatter/producer остановлены по уточнению пользователя;
их interrupted receipts сохранены, это не PASS. Уже завершённые proofs действуют
для прежних входов. Старые main348/runtime295 отчёты явно исторические.

Передtest038 остаётся только необходимая проверка затронутых команд129 при
Manual60/Original20 и проверки сборки/целостности переименований. Не запускаем
снова неизменённые player/NPC/оружие/остальные подсистемы. В этом CPU-блоке189
полных paired lifetimes/48384 Original endpoints от восьми реальных producers
и13 command profiles. Новый EXE готовится после этого общего элемента.
Полный variable FPS остальных уровней/врагов/платформ/камер/катсцен/миниигр/2D
ещё требует разработки. Игра/GPU локально не запускаются.


## Test-038 готов — единый этап предметов и эффектов

Windows EXE сохранён: `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-038.exe`.
SHA256 `6e10c54e008aa94b69d48a52744c2584baafc8ad95aea99d43f5f7249d414979`. Generation352, MSVC RelWithDebInfo; приложение/GPU не запускались.
Вmain перенесён общий Native блок motion/parent-follow/idle/scatter/reveal/rise/
shrink/spin/arc/collection стандартных111..128 и common animation129. Дискретные
Source commands/RNG/contact/quest/init/retirement сохранены. Имёна и документы:
778 описаний/447 function aliases/569 register scopes;96 canonical C/tokens/C17/ABI
прошли. Целевой pre-build suite Manual60/Original20 прошёл189 paired lifetimes/
48384 полных endpoints,13 команд до/после/внутри Native периода. Runtime350 opacity
negative сохранён. Первый352 Source-стенд некорректно разрывал только child7C;
это оставляло parent80 на освобождённую физику. Настоящий E7528 очищает обе стороны;
исправлен только fixture, полный Original endpoint/resource/coverage контроль сохранён.

Player/NPC/оружие/остальные неизменённые системы заново не прогонялись. Старые
exhaustive interrupted312/321/332/338 не объявлены PASS. Предыдущие scoped proofs
с точными inputs/origins закреплены вresearch/native-pickup-connected-stage352.json.
EXE/source/generated receipts совпали;32pins/donor прежние. Все старые EXE, ROM,
settings/saves/shared DLL сохранены; source195/test037 архивирован перед038.
Пользовательские реальные уровни/renderer в этом этапе не проверялись.

Следующий крупный блок — оставшиеся враги/объекты/платформы и их связанные события;
затем оставшиеся script cameras/cutscenes/minigames/2D. Полный variable FPS игры
ещё не завершён. Проверки нового блока — только необходимые пути после реализации,
перед следующим Windows build; многократные полные прогоны не возобновлять.


## Блок3 завершён — main354 / test-039

Windows test039 сохранён: `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-039.exe`.
SHA256 `2cca20028497f3e1a4833833d91a88d092809abd29601592ed967458e0f22827`. Native scene1 scripted policy подключена при20/30Hz:
участники сцены получают непрерывные XYZ, rotation, scale, polar movement,
jump gravity/collision drift и visual alpha. Четыре сценарные камеры обновляют
target/eye/angles/distance, orbits, local motion и следование за движущимися
участниками; world origin и eye geometry принадлежат тому же кадру.

Восемь script threads, порядок opcodes, wait/pause/Start skip/cursors, birth/free,
clip/seek, sound/text, fade и scene completion сохраняют прежний Source порядок.
Native не вызывает interpreter/spawn/audio/RNG/allocator в дополнительных кадрах:
motion closure52 functions исполняется в private guest image. Source commands
читают исходные transforms, а не последний Native sample; BB080 end захватывает
новую базу после всего исходного world update. Возврат к Original/pause использует
прежние безопасные frame/resource boundaries. Actor/physics/model identity guards
защищают заменённые объекты. Дискретный blink/physical opacity marker2A остаются Source.

Свежая targeted354:64 connected cases,416 полных Source endpoints,1131 Native
intervals,18 guards; Manual60/73 с Original20/30, actual participant/camera math,
actual interpreter wait/cut/finish, full-RAM/context Source restoration и duplicate
timestamps. Проверен также предел1ns после уже движущегося Source кадра с активными
orbit/local channels: следящая камера не применяет преждевременно весь follow offset.
ASan/UBSan прошли. Synthetic controllers и ROM constants/geometry closure:
реальные уровни/renderer не проверялись, free/text/widget boundaries считаются
стендом; прочие opcodes сохранены исходными, но весь их игровой content не прогонялся.

Каталог810 descriptions/479 function aliases/587 register scopes. Разобраны33
функции сценариев/камер, добавлены18 точных областей переменных. Fresh names354:
96 canonical C units, одинаковые preprocessed tokens, C17 syntax и прежний ABI.
Старое pending в metadata353 описывает момент подготовки; текущий результат354
подтверждён research/native-cutscene-check.json и connected-stage354 receipt.
Windows MSVC RelWithDebInfo прошёл; EXE соответствует проверенному коду/generation.
32 dependency pins/donor прежние. Предыдущие EXE/DLL/ROM/settings/saves сохранены;
Windows source352/test038 архивирован перед039. Game/GPU локально не запускались.

История отрицательных результатов сохранена: первая compile353 показала missing
clock declaration и ошибки нового CPU fixture, исправлены до исполнения.
Округлённый вверх30Hz endpoint не должен запускать следующий timer/geometry prelude
из-за остатка меньше1ns; устранён только этот residue, без лимита run duration/FPS.
Дополнительный разбор следящего eye orbit подтвердил repeated Source correction:
во fractional Native step offset перед вращением масштабируется по времени,
полный Original шаг и прочие Native owners не меняются. Замена тестового model
сначала ошибочно указывала на другой уже Native-owned live model; fixture теперь
использует отдельный replacement, а восстановление существующего владельца сохранено.

Блок3 закрыт на уровне реализации и целевой CPU/Windows сборки. Full variable FPS
всей игры ещё не завершён. Unchanged player/NPC/pickup/weapon suites не перезапускались.
Следующий блок выбирает пользователь:1 объекты/платформы;2 враги/активный AI;
4 миниигры/специальные режимы;5 2D/UI;6 общая интеграция. До выбора не начинать следующий.


## Блок5 завершён — main355 / test-040

Windows040 сохранён: `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-040.exe`.
SHA256 `78362e29bdaf6f7166f1d13cf50c4c9b3cb813879c80f1054fdf8349e02c9cce`. Общий2D/UI подключён к Native Manual: новые display lists
в title/files/intro/selection и scene1 UI для gameplay/cutscene/pause branches.
Панели считают start+quantized Source rate*(elapsed+h), screen rectangle —
четыре float bounds, title letters — четыре actual guest equation режима,
intro — alpha, selection — fixed actor sin bob/rotation и text row movement.
RT64 pose interpolation не используется. При неподключённой мировой подсистеме
UI policy оставляет world Source callbacks20/30Hz: это не заявляет Native мир.

Resident dispatcher наблюдает фактически вызванные Source UI domains; пропущенная
ветвь или пауза не продолжают math. World/script/camera/resource updates не
reinvoкe на дополнительных UI кадрах. Контроллер/repeat/answers/pages/commands,
waits/completion, saves/pak/EEPROM и text token cursors сохраняют Original порядок.
Widgets reveal/lifetime находились в phase1 draw; после первого Native draw
запоминается результат Source epoch. Дополнительный draw получает pre-state того
же Original вызова и восстанавливает post-state после построения команд. Это
сохраняет glyph reveal и Original multiplicity нескольких views. Dialogue bob,
files visual counters, text arrow blink/sound flags и final panel growth защищены
тем же владельцем.63B8 оказался полноценным arrow texture builder: geometry
исполняется каждый кадр, подавляется только повторный sound JAL444C0. UI scope
восстанавливает math fields/context до следующего Source input и reset/handoff.

Fresh targeted355:80 connected UI cases,64 Original motion endpoints,
153 Native math samples,72 repeated Original draw/command comparisons и37
ownership/pause/policy guards; Original20/30, Manual60/73, full math RAM/context restore,
ASan/UBSan. actual UI/math bodies берутся из Original ROM, reference61 не имеет
Native hooks в выбранном замыкании. Texture-info и textured glyph/rectangle/string
draw, sound request — counted fixture boundaries; реальные ресурсы/GPU/вид меню
не проверялись. Resident dispatcher/policy и UI hook sites закреплены42 точными
ROM инструкциями; unrelated world/player/NPC/weapon/pickup/camera suites не запускались.

39 разобранных UI функций получили понятные имена и описания,9 точных областей
переменных добавлены. Всего849 descriptions/518 semantic aliases/596 register
scopes. Names355:96 C units, exact canonical round trip/preprocessed tokens,
C17 syntax compilation, прежние ABI/ROM identity/register storage. Документы,
address map и соответствия старым функциям актуальны. Историческое pending в
catalog evidence фиксирует подготовку; результат подтверждён current355 proofs.

Windows MSVC RelWithDebInfo собран; EXE совпадает с checked source/generation.
32 dependency pins/donor неизменны. Source354/test039 архивирован перед040;
старые EXE/DLL/ROM/settings/saves сохранены. Локального запуска игры/GPU не было.
Первая CPU compile выявила требования RDRAM macro identifier и unused helper:
исправлены только новые UI helper functions, затем повторена лишь UI проверка
с готовым C object cache; уже прошедший names check не повторялся. Первая Windows
compile получила C1902 в служебном PDB manager ограниченного процесса; повтор
сборки в доступном окружении отдельно отражён в локальных логах. Windows
compile также выявила отсутствующий namespace двух UI policy helpers: исправлена
только их регистрация, затем та же focused fixture включает actual policy
selection всех пяти callback pairs20/30 и framebuffer rejection.


Блок5 закрыт на уровне общей UI реализации и целевой CPU/Windows сборки. Полный
variable FPS всей игры ещё не завершён. Следующий блок выбирает пользователь:
1 объекты/платформы;2 враги/активный AI;4 миниигры/специальные режимы;6 общая
интеграция. Другой блок не начинать без выбора.

---

Каталог712/377/481 и Source/mixed diagnostics сохранены отдельным этапом.
168 Source lifetime schedules и119252 Native интервала между Source callbacks
сохраняют предметы118/123/127 и шесть effects130; Native trajectory ещё не
подключена. Diagnostic bank flags не означают session/presentation/GPU test.
Все96 canonical C units и ABI идентичны217;10 current reports соответствуют234.
Fresh ordinary camera component9571 проверяет прежний camera code.

Isolated FD220 full прошёл5088 Source/720 Native/34464 frozen comparisons.
Полный224 с Source-end camera-shake fix ещё идёт; этот runtime не включён вmain.
Actual Source room barrier имеет permanent isolated231 CPU check на двух banks,
с остановкой до Original destructors и новой загрузки. Следующий connected
checkpoint ожидает224 full. Native rewards/effects, другие levels/enemies/
platforms, script cameras/cutscenes, minigames/2D иобщая интеграция остаются.
Нового Windows EXE нет; test037 иNPC B fix сохранены. Git — основное сохранение.

---

## Продолжение v193 — сюжетные городские NPC прошли общую проверку

Story100 семей13/14/15/16/17/20/21/22, вторичные модели и связанные kind6,
а также специальные состояния200/201 kind21 подключены к Native.
Все семь formal reports соответствуют generation193 и текущим source SHA:
679 Original functions/regions;1152 actual constructor/profile comparisons,
2304 story Source/1152 Native/768 rejection/15360 exact frozen189
RAM/context/heap/resource/command/model-count comparisons;768 special Source/
192 Native/96 rejection/5280 exact comparisons и288 dense Original counter
samples. Прежние player/weapon/damage/death/recovery/door/surface/world и
городские patrol/dialogue/linked группы проверены в той же общей сборке.
ASan/UBSan/LSan и stability193 прошли на24 настройках20/30Hz и60..360FPS.

Исполняются настоящие конструкторы и descriptor parser. У13/14/16 generic
constructor уже создаёт две model groups;13/14 profile1 прикрепляет group1
к кости19 group0 черезA5F0. A2E8 задаёт флаг группы, а не создаёт модель.
Проверены20-бонная synthetic hierarchy, все профили0/1/2, два размещения в
пуле и настоящее раздельное освобождение моделей/ресурсов. У16/profile0
actual sprite42=1; основная диалоговая проверка теперь покрывает именно его.

Kind6 создаётся первым story100 callback, а не конструктором родителя.
Native принимает лишь настоящие взаимные связи, порядок индексов, живые
model lifetimes, правильный overlay/table callback и согласованные C9.
Связанная семья использует возраст родителя при появлении на73/173мс,
паузе, возврате из Source-состояния и полном пересоздании.

Special201 вычисляет rate5C, но Original не применяет здесь постепенный
возврат. Сохранены его мгновенные yaw snaps и возврат в200 на шестом такте.
Изменение busy/yaw внутри одного периода может пересчитать решение,
но не расходует второй дискретный такт. Контролируемое отключение защиты
поймано dense Original oracle даже на Native60; версия с ошибкой отдельная.

643 описания/308 semantic aliases/402 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI; документация обновлена вместе с
именами. main/HEAD,donor,32 pins и frozen035/source181 сохранены.
Игра/renderer/GPU не запускались; эта проверка использует synthetic assets,
явные sprite2/audio/UI boundaries и настоящий guest lifecycle.

В отдельной candidate194 разбирается slot09 ROM1398A0: актёры24..28,
их scene/story guards, контакты и создаваемые объекты. Прочие уровни,
платформы, script camera/cutscenes, миниигры и2D остаются в работе.
Полный variable FPS не завершён; завершение городского блока не завершает
общую задачу. EXE выпускается только после законченного крупного элемента.

---

## Продолжение v192 — диалоги восьми городских NPC прошли общую проверку

Диалоговые состояния13/14/15/16/17/20/21/22 подключены к Native вместе с
прежними player/weapon/damage/death/recovery/door/surface/world группами.
Все семь formal reports соответствуют generation192 и текущим source SHA:
653 Original functions/regions,3072 Source и2304 Native edge schedules,
384 overlay rejections,4608 actual contact-producer comparisons и20736
exact frozen189 full-RAM/context/heap/resource/command comparisons.
Проверены19712 промежуточных значений обоих углов на24 настройках20/30Hz
и60/73/120/144/165/360FPS. ASan/UBSan/LSan и stability192 прошли.

Поворот к игроку и возврат используют отдельные исходные предложения для
угла актёра и физического угла: Source может изменить оба в одном callback,
а затем вернуть только угол актёра. Сохраняются Original переходы, команды,
клипы, busy-флаг и особенность нулевой скорости возврата. Проверены пауза,
возврат из Source-состояния, настоящее пересоздание модели, внешняя запись
углов, перепривязка клипа, рождение на73/173мс и границы signed wrap.
Контролируемый отрицательный тест с одним общим углом обнаружен даже на60FPS.

633 описания/298 semantic aliases/392 register scopes сохраняют canonical C,
preprocessed tokens и ABI. Документация и ссылки на разобранные функции
обновлены. main/HEAD,donor,32 pins и frozen035/source181 сохранены.
Новый EXE не собирался; игра/renderer/GPU не запускались.

Сюжетные состояния100, дополнительные модели и состояния200/201 ещё проходят
отдельную проверку в candidate193. Полный variable FPS остаётся в работе:
остальные уровни, платформы, script camera/cutscenes, миниигры и2D не завершены.

---

## Продолжение v191 — связанные городские актёры прошли общую проверку

Реальное создание по дескрипторам, распределение связанной пары, контактный
обработчик и раздельное освобождение моделей проверены вместе с прежними
player/weapon/damage/death/recovery/door/surface/world группами:629 Original
functions/regions,48 constructor schedules,1008 Source и720 Native schedules,
288 ownership rejections,308 промежуточных углов,6768 exact frozen189
RAM/context/heap/resource/command comparisons и384 actual contact comparisons.
Все семь formal reports соответствуют generation191 и текущим source SHA;
ASan/UBSan/LSan и stability191 прошли на24 настройках20/30Hz и60..360FPS.

Поворот к игроку и обратный поворот обновляются между исходными тактами,
а команды и переходы сохраняют Original cadence. Детали контакта и диалога
разобраны для linked18/19; story-вариант18/6 использует общий возраст семьи.
Получатель19 читает только окончательную публикацию родителя после base
callback. Внутренние привязки клипов родителя не являются командами ребёнку.
Отрицательная проверка выявила потерю повторной публикации одного и того же
клипа одновременно на Native60 и более высокомFPS: сравнение только между
частотами скрывало ошибку. Счётчик публикаций и сравнение с Original исправили
и проверили этот случай. E656C снимает взаимные ссылки, EA4C освобождает одну
модель; автоматического рекурсивного удаления пары нет.

609 описаний/274 semantic aliases/352 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI. Каталог generation191 содержит
доказательство кандидатной проверки; настоящая общая проверка теперь связана
в research/native-city-linked-check.json. Исправлены отставшие описательные
поля player report с сохранением SHA исходного отчёта, stdout и проверенного
checker source. main/HEAD, donor,32 pins и frozen035/source181 сохранены.
Новый EXE не собирался, игра/renderer/GPU не запускались.

В отдельной candidate192 подготовлены диалоговые состояния восьми других
городских NPC. Их ещё не проверенная реализация не перенесена в main.
Диалоги этих NPC, secondary/story/200 состояния, остальные уровни и платформы,
script camera/cutscenes, миниигры и2D остаются в работе. Полный variable FPS
не завершён; отдельный connected блок не является завершением всей задачи.

---

## Продолжение v190 — восемь городских patrol NPC прошли общую проверку

Полный connected check исполнил611 Original functions/regions вместе с
прежними player/weapon/damage/death/recovery/door/surface/world группами.
1536 Source/1152 Native schedules,192 overlay rejections,1232 intermediate
position samples и11904 exact frozen189 RAM/context/heap/resource/command
comparisons прошли на24 настройках. Все семь formal reports совпали с
generation190 и текущими SHA; ASan/UBSan/LSan и stability190 прошли.
597 описаний/262 semantic aliases/343 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI. Документация синхронизирована;
main/HEAD, donor,32 pins и frozen035/source181 сохранены. Новый EXE не
собирался, игра/renderer/GPU не запускались.

В отдельной кандидатной191 копии реальный descriptor parser, generic
constructor, linked allocator и раздельное освобождение18/19 прошли48
Source schedules на24 настройках. E5DF4 начинает поиск с parent index:
ранний свободный слот не делает leg раньше parent. E656C лишь снимает
взаимные ссылки, а EA4C освобождает одну модель; рекурсивного удаления нет.
Synthetic inactive slots имеют explicit resource owner=-1, потому что
scene_object_init не очищает это поле. Native command-period/visible-yaw
реализация191 подготовлена; её differential proof продолжается и в main
она ещё не перенесена. Dialogue/attached/story состояния остальных NPC,
реальные комнаты/уровни, script camera/cutscenes, миниигры и2D остаются.
Полный variable FPS не завершён.

---

## Продолжение v190 — ожидание и ходьба восьми городских NPC в main

Подключены Native state1/state2 для kinds13/14/15/16/17/20/21/22. Реальный
FSM сохраняет private wait40, одноразовые clip0/1/2 и исходные команды
разворота один раз за Original период; общие перемещение и model time
обновляются на каждом Native интервале. X/Z используют один float baseline
на период с повторным захватом после явной записи позиции/скорости, чтобы
накопленная погрешность не сдвигала точную границу маршрута. Model rebind
не дублирует private counter; новая model lifetime получает новый clock.

Изолированная190d проверка прошла1536 Source/1152 Native/192 overlay rejection,
1232 intermediate position и11904 exact frozen189 Original RAM/context/heap/
resource/command comparisons на24 настройках. Разобраны все восемь разных
осей, границ и direction flags. Negative guard-off даёт count9 вместо4;
внутренние clip-boundary splits тоже не должны ускорять ожидание. Source
snapshot каждые3 периода имеет точное ns-время; floor1/3ns ещё до discrete
end counters, а ceil уже исполняет начало следующего периода. Все clip
commands дополнительно сравниваются по исходным rational timestamps.
Oracle обязан восстанавливать и сравнивать actual clip-channel heap/refs
вместе с RAM, иначе двойной проход некорректно повторно освобождает buffers.

В main597 описаний/262 semantic aliases/343 register scopes; документация
синхронизирована. Полный connected190 check выполняется; последний совпавший
полный архив189 сохранён. Нового EXE нет, frozen035/source181 прежний;
игра/renderer/GPU не запускались. Dialogue/attached/story состояния этих NPC
и связанная18/19 ещё Source-owned. Разбор real descriptor constructors/
attachment ведётся дальше; остальные комнаты/уровни, NPCs/platforms,
script camera/cutscenes, миниигры и2D тоже остаются. Полный variable FPS
не завершён.

---

## Продолжение v189 — диалоговые helpers75/76 и cat77 прошли общую проверку

Полный connected check исполнил545 Original functions вместе с прежними
player/weapon/damage/death/recovery/door/surface/world группами.864 Source,
144 Native schedules,72 отказа overlay и1032 промежуточных наблюдения yaw
новой диалоговой группы прошли на24 настройках. Все семь formal reports
совпали по generation189 receipt и текущим SHA. ASan/UBSan/LSan прошли;
main/HEAD, donor,32 pins и frozen035/source181 сохранены.597 имён/262 aliases
ещё не в main: это отдельно подготовленная кандидатная190 группа городских
patrol callbacks13..17/20..22; её CPU-проверка продолжается. В main остаются
573 описания/238 aliases/335 scopes с точным canonical C и ABI.
Нового EXE нет; игра/renderer/GPU не запускались. Остальные состояния NPC,
связанная модель18/19, комнаты/уровни, script camera/cutscenes, миниигры и2D
ещё не завершены. Полный variable FPS остаётся в работе.

---

## Продолжение v189 — helper75/76 и cat77 в main, focused PASS

Настоящие callbacks/contacts летающих помощников75/76 и кота77 подключены
к Native времени.864 Source/144 Native schedules,72 отказа владения overlay
и1032 промежуточных наблюдения yaw прошли на24 настройках. Проверены реальные
descriptor initializer, достаток/нехватка предметов, принятый/отклонённый ответ,
повторные контакты, byte counters, model clips, покачивание, поворот/возврат,
pause/rebind/external writes, constructors73/173ms, AI suppression/resume
и настоящий destructor. Полный remaining heap/resource references совпадает.

Actual initializer устанавливает pending answer1; итог ответа поступает через
наблюдаемую границу ещё не переведённых dialogue progression/input. Внешний
busy release после rational Source boundary нельзя записывать за долю ns до
неё: только этот after-boundary event flush использует ceil. Обычная геометрия
сохраняет floor. Cat descriptor зависит от выбранного персонажа801B374A;
имена game-counter сохраняют исходное signed-byte переполнение/ограничения.

573 описания/238 смысловых aliases/335 register scopes; names189 подтвердил
точный canonical/preprocessed C и ABI для96 C units. Документация иaddress-map
обновлены. Model/loop/simulation/frame-time/overlay189 прошли; stability189
сохранила main/HEAD, donor,32pins и034. Full connected player189 выполняется;
последний совпавший полный архив188 сохраняется. Windows035/source181 прежний,
новогоEXE нет; игра/renderer/GPU не запускались. Исследуется следующая группа
городских NPC13..22, включая связанную модель18/19. Остальные комнаты/уровни,
NPCs/platforms, script camera/cutscenes, миниигры и2D ещё в работе.
Полный variable FPS не завершён.

---

## Продолжение v188 — возвратный поворот исправлен и прошёл общую CPU-проверку

Новый отрицательный сценарий подтвердил пропуск delay-slot записи yaw между
Native кадрами:350 вместо358.219 на73FPS. Теперь запись сохраняется, а state1
переход остаётся на конце исходного периода.1152 orbital Source schedules,
480 Native schedules и1376 промежуточных наблюдений угла прошли оба
направления и переход через360 градусов. Городские288 Source/96 Native/48
ownership rejection и все прежние player/weapon/damage/death/recovery/door/
surface группы прошли вместе с исправлением. Все семь generation188 отчётов
совпали по receipt и текущим SHA; ASan/UBSan/LSan и stability188 прошли.
562 описания/227 смысловых aliases/330 register scopes сохраняют canonical C,
preprocessed tokens и ABI для96 C units. Документация синхронизирована.

В изолированной build-local подготовке189 настоящие helper75/76 и cat77 прошли
864 Source/144 Native/72 ownership rejection schedules и1032 промежуточных
yaw observations на24 настройках. Исполняются actual contact/descriptor/clip/
answer/counter/destruction цепочки; outcome progression/input и sound execution
остаются наблюдаемыми границами. Каталог189 подготовил573 описания/238 aliases/
335 scopes. Перенос189 в main и его connected check выполняются следующим шагом.
Игра/renderer/GPU не запускались, новыйEXE не собирался; frozen035/source181
сохранён. Другие NPCs/platforms, комнаты/уровни, scripted camera, катсцены,
миниигры и2D ещё в работе. Полный variable FPS не завершён.

---

## Продолжение v187 — городские7/9 прошли общую CPU-проверку

Все семь отчётов player/model/loop/simulation/frame-time/names/overlay совпали
с generation187 и текущими SHA исходников. Полный player check исполнил534
Original functions/regions вместе с прежними группами движения, оружия, урона,
смерти, восстановления, дверей и поверхностей. Новая городская группа прошла
288 Source/96 Native schedules и48 отказов владения overlay. ASan/UBSan/LSan
прошли; runtime overlay guards проверены для49 диапазонов и5 ожидаемых отказов.
Сохранены562 описания/227 смысловых aliases/330 register scopes; canonical C,
ABI и preprocessed tokens для96 C units совпадают. Документация синхронизирована.
main/HEAD, donor,32 dependency pins, frozen Windows035/source181 сохранены.
Новый EXE не собирался; игра/renderer/GPU не запускались.

Отдельный новый сценарий ненулевого возвратного поворота выявил пропуск записи
угла в delay slot на промежуточных Native кадрах orbital71..74. Старые сценарии
этого не покрывали. Исправление и проверки обоих направлений через360 градусов
подготовлены для188; данный архив187 сохраняет предыдущий проверенный объём.
Разбор helper NPC75/76 и cat77 продолжается. Реальные комнаты/уровни, другие
NPCs/platforms, script camera/cutscenes, миниигры и2D ещё не завершены.
Общий variable FPS остаётся в работе.

---

## Продолжение v187 — город7/9 focused PASS, общий connected check выполняется

Городские dora3_city_kind7_state_dispatch_8022D938 и
dora3_city_kind9_state_dispatch_8022DBB0 прошли288 Source schedules и96 Native
off-grid schedules на24 настройках (20/30 Source,60/73/120/144/165/360 render).
48 ownership rejection scenarios подтвердили, что без verified .level1_slot1
объект/модель не обновляются Native, а настоящий Source dispatcher сохраняет
владение. Отдельная проверка исполнила реальные runtime DMA/load/unload guards
для всех49 ROM overlay ranges, подмены того же VRAM, частичной перезаписи,
родительских зависимостей, unload/boot reset и5 ожидаемых отказов81/85.

Исполняются настоящие city contact mode4 producers (и skipped modes0/1/2/3/5),
повторные контакты, подготовка dialogue descriptor/state, pool permission,
player scripted state66, белый fade command, sprite pool bind/delay/release,
16-update retirement и Native constructors на73/173ms. Первая CE0 sprite bind
принадлежит началу Original sprite периода; выход CD4 и события counter — концу.
Проверка heap учитывает ровно реально заменённые Original player model buffers
60/6C/70; все city allocations освобождаются, geometry/остальные allocations
совпадают. Ошибки fixture со статическим B4 contact list, resource id0 и смешением
player/city sprite counters исправлены без снятия resource/destructor assertions.

Теперь562 описания/227 смысловых aliases/330 register scopes; names187 подтвердил
точный canonical round trip, одинаковые preprocessed tokens и C17 syntax для96
C units. Документация имён обновлена. Source/render timing и ABI не переименованы.
Общий player187 и model/loop/simulation/frame-time checks идут; последние полные
совпавшие отчёты/архив185 остаются историческим проверенным checkpoint.
Frozen Windows035/source181 и общие settings/saves сохранены; нового EXE нет.

Контактная collision selection, продвижение/верстка/input диалога и sound queue
ещё не проверены (initializer использует наблюдаемый sound queue failure).
Реальная room construction/DMA, другие NPCs/platforms, уровни, scripted camera,
катсцены, миниигры и2D остаются в работе. Полный variable FPS не завершён.
Игра, renderer и GPU не запускались. Продолжается следующая группа.

---

## Продолжение v186 — городская группа7/9, CPU-проверка выполняется

В текущую main рабочую копию подключены dora3_city_kind7_state_dispatch_8022D938
и dora3_city_kind9_state_dispatch_8022DBB0. Первый сохраняет проекцию скоростей
и переход со сменой спрайта на Source границе; второй —16 Source обновлений,
абсолютный timestamp события15F, остановку пулов/игрока и реальное удаление.
Native ownership требует live callback и verified loaded ROM .level1_slot1:
другой overlay с тем же VRAM не получает эти правила. Новые Native-born модели
получают собственный период, команды после счётчика сохраняют мировое время.
Готовится Source/Native доказательство, feature ещё не объявлен проверенным.

Fixture исполняет реальный sprite constructor, frame delays и actor/model
retirement на синтетических assets; actual metadata kinds7/9 имеет sound0
и drop3, поэтому эта ветвь не создаёт дополнительных drop objects. Добавлены
12 смысловых функций,9 register scopes; теперь554 описания/219 aliases/321 scopes.
Документация имён синхронизирована, names186/connected city check идут.
Последний полный совпавший checkpoint —185 (архив v185-connected-world-objects);
его успешные отчёты нельзя приписывать новым изменениям186. Frozen EXE035/source181
сохранён, новой сборки не делалось. Игра/renderer/GPU не запускались.
Полный variable FPS для остальных NPCs/platforms, уровней, скриптовых камер,
катсцен, миниигр и2D остаётся незавершённым; работа продолжается.

---

## Продолжение v185 — общий связанный CPU PASS

Все шесть свежих отчётов player/model/loop/simulation/frame-time/names совпали
с generation185 и текущими SHA исходников. Полный player check исполнил
515 Original functions/regions и прежние группы движения, камеры, оружия,
1392 lethal/death/respawn,144 post-ready и двери/поверхности вместе с новыми
96 Source/96 Native простых объектов и960 Source/480 Native orbital schedules.
ASan/UBSan/LSan прошли. Проверка стабильности сохранила main/HEAD, donor,
32 dependency pins и034; frozen Windows035/source181 уже сохранён ранее.
542 описания/207 смысловых функций/312 областей переменных; все96 C units
сохраняют canonical/preprocessed C tokens и ABI. Документация синхронизирована.
Игра, окно и GPU не запускались; новая сборкаEXE для этого шага не делалась.

Разбирается следующая городская группа7/9: проекция скоростей, ожидание со
сменой спрайта и событие после16 обновлений с удалением объекта. Новые имена
и код подготовлены отдельно; в данном checkpoint185 их Native ownership нет.
Сюжетная ветвь orbital dialogue, другие active NPCs/platforms, actual room
construction/levels, scripted camera/cutscenes, minigames и2D ещё в работе.
Общий variable FPS не завершён. Архив сохраняет проверенный связанный185.

---

## Продолжение v185 — движение объектов по окружности и собственное время появления

Подключены dora3_orbit_object_kind71_74_update_801EEFC8 и его состояния:
dora3_orbit_object_state1_update_801EECF4 сохраняет круг3°/Source период,
dora3_orbit_object_state2_update_801EEE80 — поворот30° и возврат.
Вертикальная волна10° использует исходный sin/cos и предыдущее значение угла.
dora3_orbit_object_contact_command_801EEC70 проверяет настоящий режим4;
в тесте комната1 исключает неподтверждённую сюжетную ветвь dialogue producer.
Общие перемещения сохраняют полный velocity offset после абсолютного XYZ,
а инкрементальные оси и остальные объекты получают Native dt.

Focused CPU PASS:96 Source/96 Native сценариев простых objects6/80,
960 Source и480 Native сценариев orbital71..74,912 full-RAM Original
сравнений; все24 настройки, ASan/UBSan/LSan. Есть движущаяся цель,
contact/return, AI disabled/resume, pause73/resume173, внешние углы,
rebind и настоящий destructor/constructor с повторным адресом модели.
Новое создание в73/173ms сопоставлено с исходной логикой от момента рождения.

Отрицательные результаты: moving-target diagnostic подавал новую команду после
ceil, который уже начинал следующий период на долю наносекунды. Исправлена
граница подачи команды, допуски не увеличивались. Реальное off-grid создание
показало потерю дробного short-angle на общем Source такте: x=-490.413
против -490.041. Исправлено собственным периодом newborn actor; planner
делит обновления на его точных границах, pause сохраняет его возраст.
Состояние привязано к actor/physics/model/lifetime; следующему pool slot
и player возвращается общий clock. Команды сохраняют абсолютный world timestamp.
Первый birth diagnostic отдельно выявил ошибку восстановления heap в fixture:
не следует перевязывать старую модель перед созданием новой; исправлено без
ослабления проверки освобождений. Source Original ветвь не менялась.

Имена:542 описания,207 смысловых функций,312 областей переменных;
включены проверенные FPR union aliases. Все96 C units имеют точный canonical
round trip, одинаковые preprocessed tokens и проходят C17 syntax проверку.
Model/simulation/frame/loop проверки185 прошли; full connected player185 идёт.
Основной connected player checkpoint пока181; test035/source181 сохранён.
Новый Windows EXE на этот отдельный шаг не собирался. Полный variable FPS
для всей игры ещё не завершён; следующие группы — active city NPCs/platforms,
реальное построение уровней, scripted camera/cutscenes, minigames и2D.

---

## Продолжение v182 — простые active objects6/80

Подключены два дополнительных callback: kind6 однократно включает clip0 вstate1,
kind80 имеет отдельный Original no-op. Движение/model/opacity/timers выполняются
на Native dt через общий dispatcher, live callback identity проверяется.
Focused world CPU PASS:96 Source сравнения с frozen unpatched dispatcher61,
96 off-grid pause73/resume173/rearm273 и912 full-RAM Original сравнения;
все24 settings, ASan/UBSan/LSan. Старый mock Source AI теперь synthetic kind7,
чтобы actual6 проверялся без имитации. Первый diagnostic неверно фильтровал
trace clip entry как70A50; Original core trace8002A328 имеетkind1000. Fixture
исправлен по generated Original, runtime и допуски не ослаблялись.
537 описаний,202 смысловых имён,308 областей переменных; names182/docs PASS.
Полный связанный player checkpoint остаётся181; новое покрытие пока focused world.
Windows035/source181 frozen и сохранён, новогоEXE для этого малого шага нет.
Разобраны функции orbital71..74: circle3°, vertical10°, yaw30°/возврат; перевод
и connected tests продолжаются. Остальная общая цель variable FPS не завершена.

---

## Windows test-035 — verified181, 2026-10-02

Общий EXE после крупного связанного блока действий/урона/восстановления/
смерти/возрождения/дверей/поверхностей скомпилирован MSVC и сохранён:
D:/Games/DoraRecomp3/testbuild/Dora64-3-test-035.exe.
SHA256: d703851e214859d70a4bd074d7989a891cf2e1bb1fe2455ede0825ea4e88ec6b.
Шесть текущих CPU reports, generation181, donor и32pins сверены.
ROM/port settings/saves/shared DLLs/034 сохранили прежние SHA.
Game/window/renderer/GPU не запускались; проверки прохождения не выполнялись.
Windows source frozen181. Разработка продолжается с active objects/AI;
общий variable FPS ещё не завершён. Новый EXE на каждую малую правку не делать.

---

## Продолжение v181 — двери и специальные поверхности: общий CPU PASS

Подключены дверь59 и поверхности60..62 через настоящий producer проверки
поверхности. Native выполняет движение/столкновения/анимацию и вращение;
Source command phases сохраняют выход/вход, счётчики и однократные события.
F5 сохраняет подъём и вращение; F4 — однократный толчок при уходе и исходный
перезапуск клипа во время опоры; F6 — spin и heading45°×orientation.
Door59 реально приостанавливает пулы10..29/30..229, записывает команды камеры,
запускает white fade/выбор следующей сцены или выдаёт room request на count10/16.
При запросе старое движение прекращается до следующего владельца комнаты.

Fresh181:503 Original functions/regions;216 Source surface schedules,
144 off-grid Native constructor/reentry/model-rebind/yaw/exit schedules,
96 door pause/camera-command/fade/room-request schedules, оба camera modes,
оба Source dividers,60/73/120/144/165/360FPS. Все прежние группы, включая
1392 lethal/death/respawn и144 post-ready reacquisition, прошли вместе.
ASan/UBSan/LSan PASS. Player/model/loop/simulation/frame-time/names отчёты
совпадают с181 generation receipt и текущими SHA. Donor/32pins/034 стабильны.

531 function descriptions,196 semantic aliases,307 register-role scopes;
96 generated C units сохраняют точный canonical C/preprocessed tokens и ABI.
Новые роли включают pause диапазона actors, scripted camera commands,
special door target, heading input и surface mode flags. Документация обновлена.

Отрицательный178 выявил повторный F4 clip reset на каждом Native dt: настоящий
producer повторяет entry61 лишь по Source clock. Добавлены guarded ctor phase
и suppression по actor/physics/model lifetime/run/tick. Допуски не расширены.
Door fixture исправлен по Original: F2 включает flight ещё до callback; grounded
doorwayF1/FC. Выбор scene1E/5A изB8ED0 зависит от scene-state, не weapon field.
Full181 выявил остаток death barrier в новом door test; fixture ограничивает
его death-watch, сохраняя реальные handoff проверки. Общая проверка повторена.

Комнаты/модели синтетические. Actual script camera advancement, room DMA/decode/
construction, внешний producer49, active NPC/AI/platforms, остальные сцены,
катсцены/миниигры/меню/2D остаются в работе. Общий variable FPS не завершён.
Проверки не заменяют прохождение. Игру/окно/renderer/GPU не запускала.
Следующий крупный Windows EXE подготавливается после этого connected checkpoint;
034/v130 пока сохранён. Новые ветки/reset/clean/commit/push не выполнялись.

---

## Продолжение v177 — постоянные имена и описания декомпилированного кода

По запросу пользователя понятные имена закреплены в recomp/decomp-symbols.json.
525 функций имеют постоянные описания,190 адресных имён получили смысловые
псевдонимы,305 подтверждённых областей регистров получили имена переменных.
Генератор обновляет определения/вызовы и восстанавливает прежние ABI symbols
через decomp_names.h. Register storage, ROM/overlay identity, function pointers,
Original инструкции и диагностические номера строк сохраняются.
Неизвестные роли не угадываются; один регистр может иметь разные имена по областям.

Fresh177 naming PASS:96 C units, точный canonical round trip и одинаковые
preprocessed C tokens, C17 syntax compilation. Canonical C, declarations,
overlay tables и runtime sources совпадают с проверенным175 checkpoint.
Новые полные player/model/loop/simulation проверки не повторялись ради имён;
их fresh отчёты относятся к175. Frame-time177 и stability177 выполнены отдельно.
Описание роли не означает полное игровое покрытие. Native59..62/room loader
и другие ранее указанные незавершённые границы остаются незавершёнными.

Справочники: [функции](decomp-symbols.md), [переменные](decomp-variable-roles.md).
tools/update_decomp_docs.py обновляет справочники/address-map/старые полные имена
в документации; --check проверяет согласованность. Порядок для будущих разборов
закреплён в AGENTS.md. generated Original C/ROM/resources не добавляются вGit.
main/HEAD/donor32pins/Windows034 сохранены. Никакого EXE/Windows sync/game/GPU.
Общий variable FPS продолжается; этот пункт не является завершением проекта.

---

## Продолжение v175 — смерть, возрождение и повторное подключение персонажа: CPU PASS

Завершена связанная проверка lethal входов32/35/37/40/43/46/47/48/49,
состояний смерти50..56, fade, game-over и освобождения старых моделей.
Actual C9BE0 выполняет создание ресурсов/моделей всех шести персонажей;
после room-ready actual67/Ground возвращает Native обновление новых моделей.
Команды после resource barrier сохраняют rational timestamp; старое движение
отменяется по invocation epoch. Байтовые записи здоровья теперь допускают
настоящие unaligned status pointers. Original20/30 сохраняет поведение.

Fresh175: пять отчётов с совпадающими source/receipt SHA. Full player489
Original routines/regions:1392 connected lethal/death/respawn schedules,
144 post-ready reacquisition плюс все прежние группы. ASan/UBSan/LSan PASS.
Frame-time:655360 Original fade comparisons и12 fractional command settings;
loop: FIFO/two-bank/context/resource barriers; model и simulation PASS.
Комнаты/ресурсы синтетические; actual room DMA/decode/level construction,
FX allocation/visuals и внешний producer49 остаются непроверенными границами.

Это завершённый CPU checkpoint блока, не завершение общего variable FPS.
Двери59/поверхности60..62, active AI/платформы, сценарные камеры, катсцены,
миниигры, меню/2D и реальные уровни остаются в работе. Windows034/v130 frozen;
новогоEXE/Windows sync нет. Игру/окно/renderer/GPU не запускала.
main/HEAD, donor и32 dependency pins сохранены. Исходники175 архивированы.

---

## Продолжениеv174 — смерти/возрождение и lethal входы; полная проверка блока ещё идёт

171 focused PASS:528 Source/Native пары на24 settings, actual50..56,
flight55, все6 healthy51 respawns. Actual C9BE0/C9364/C9794/C93D4/C97E8:
старые модели/списки освобождаются, новые реально создаются; реальные
13-node model trees и команды видимости оружия5..12, resource reference
counts, повторное освобождение и reuse guest heap addresses проверены.
Actual BA084 room request имеет тот же точный rational command timestamp,
что C9BE0, даже после отмены Native session. Original first-state commands
продолжаются; старое movement/model second body больше не возобновляется.

172 focused PASS:тот же528 matrix плюс144 шестиперсонажных post-ready
Original67/Ground -> Native reacquisition trajectories. Pending room request
EC=1/2, secondary request7118 и countdown70F0 не разрешают Native acquisition.
Actual B8FB4 отсчитывает hold; actual C9C58/C97E8/camera reset и полный
Original player/origin/camera update возвращают управление новым моделям.
Room DMA/resource decode/real level construction — явная synthetic boundary;
полная загрузка реальной комнаты этим стендом НЕ подтверждена.
Перед B9FDC/room retirement добавлен bank/resource barrier.

173 Native lethal eligibility расширена на32..49/48; новая матрица включает
actual health loss C0738, damage ctors, real C2978 producer35/48, реакции,
смерть/fade/scene cleanup. Первые проверки — negative, не полный PASS.
173 uncovered:байтовый status at801B361B+2*character ошибочно проверялся как
aligned word pointer, поэтому real health records не допускали healthy
damage/hover Native.174 разделяет byte range и aligned structural pointers;
проверка идёт с настоящими unaligned character records. Health>0 для63
сохраняется; external49 producer по-прежнему не исследован.

Последний полный checkpoint166. Fresh169 frame-time/loop PASS были до
последующих изменений; не использовать как подтверждение174 дерева.
Windows034/v130 frozen, новогоEXE/sync нет; игру/window/renderer/GPU не
запускала. Продолжать общий variable-FPS scope, без ожидания playtests.

---

## Продолжениеv169 — подключение смерти50..56 и точное время fade; блок ещё не завершён

Добавлены Native second/collision/model bodies50..56 и один первый command
closure в Source-периоде. Actual ctors сохраняют new-second/next-first order.
Перед EF5B0/actor cleanup подключён bank/resource barrier, до деструкторов.
Новые команды fade получают rational SourceTimestamp с точностью1/60ns;
sampling render использует момент обновления gameplay, а не более позднюю
wall-clock стоимость подготовки кадра. Barrier сохраняет время до cancel.
Original20/30 вне Native сохраняет исходные функции.

Connected focused169:336 пары Source/Native сценариев50..56, живой другой
персонаж либо game-over,24 settings(20/30Hz,камеры12/2,60/73/120/144/165/360).
466 Original routines. Каждый Native game-over выполняется после подключения,
actual модели реально уничтожаются через actor cleanup. Все промежуточные
счётчики/команды exact; geometry tolerances unchanged; ASan/UBSan/LSan.
Движение и fade после передачи следующей сцене не сравниваются как состояние
уже освобождённого владельца. Банки/очередь/context проверяются отдельно
реальным loop, player fixture имеет явный checked barrier stand-in.

Fresh169 frame-time PASS:655360 Original fade comparisons плюс12 rational
command/reversal settings с ненулевым origin; fresh loop PASS:full Original
message/frame loop,270 white-fade builds, FIFO/two-bank/context resource barrier.
Full-player/model/simulation отчёты169 ещё не выполнялись; последний полный
проверенный checkpoint166. Возрождение51 при ненулевом здоровье и lethal
входы через32..49/48 пока остаются прежними незавершёнными границами.
Никакого EXE/Windows sync/game/window/renderer/GPU;034 остаётся frozen.
Дальше: actual respawn/model recreation, lethal paths и другие системы scope.

---

## Продолжениеv166 — здоровое восстановление49 и управление63, без EXE

Подключена здоровая цепочка control2C=1:49 count21 ->63 clip35/rearm30,
разгон/торможение и поворот actual C8118/C1DE8; A меняет90/270 сразу,
скорость подъёма/спуска10. X/Z используют согласованные kick/drift перед
Original collision, Y от guarded constant-velocity baseline с одним float
округлением. Actual solver/external writes/model run/lifetime/gaps rebases.
B очищает2C, вызывает настоящий fall, landing и полный ground return.

49 first сохраняет входящий count своего Source-периода: собственный
second increment не разрешает ранний переход. Counter guard сохраняет
одно увеличение, actual ctor и внешняя запись reset переустанавливают его.
Повторное вооружение30 не декрементируется хвостом собственного периода.
В fall9 первое чтение actual C0C7C flags сохраняется до следующего Source
периода; дополнительный дробный шаг не запускает landing на долю нс раньше.
Это исправило negative165 state1 vs10 на длительности клипа60 без допусков.

Fresh166:144 Source +144 Native schedules,24 settings:20/30Hz,
камеры12/2,60/73/120/144/165/360FPS. Original comparison через реальные
полные player/world-origin/camera updates; каждая третья Source boundary
имеет точный integer-ns timestamp. Native73/173/273ms ctor/re-hit/rebind/
position-yaw-speed writes, A lift/release, B cancel and complete landing;
21/23 fields, exact state/counters/health/control/command counts. Source49
rearm30 и1, ctor63 stationary/steered, yaw359 and actual re-entry tested.
Все прежние connected группы прошли;435 Original routines/regions;
ASan/UBSan/LSan. Четыре fresh отчёта совпадают с receipt/source SHA.

Первичный внешний producer, который выбирает49, пока не проверен: новые
Native тесты вызывают actual health loss C0738 и actual ctor49 в заданный
момент. Это не утверждение о прохождении этого удара в реальном уровне.
Комната/ресурсы синтетические; настоящий visual/gameplay проверяет пользователь.
Предыдущие границы FX/model3/sprite5 сохраняются. Windows034/v130 EXE
не изменён; нового EXE/Windows sync нет. main/HEAD/donor32deps сохранены.
Игру/окно/renderer/GPU разработчица не запускала. Verified166 source archived.

Полный variable FPS продолжается: смерти/возрождение и прочие действия,
active AI/платформы, сцены, сценарные камеры, катсцены, миниигры и2D.
Не ожидать playtests и не собирать EXE на этот промежуточный шаг.

---

## Продолжениеv163 — специальные действия64..67, без нового EXE

Подключены64: три ветки команд персонажа (0:48/318;5:111/336;прочие:0),
полная анимация/отмена/возврат и настоящая цепочка создания, привязки костей
и освобождения model3 на синтетических ресурсах.65 остановка/отмена;66
поворот по краткому направлению с исходной зоной30 градусов и постоянная
скорость;67 одноразовая настройка и десять исходных шагов восстановления,
скорость5, ground/flight и немедленный возврат по аргументу1. В ветках66/67
полёт использует фиксированную проекцию, без чужой команды ускорения.

Счётчик ожидания обновляется до выбора первого callback, один раз за
Source-период. Собственная смена состояния/клипа не повторяет его и не
сбрасывает раньше времени. Реальный вход из idle298->299->300 проверен.
Независимый reference на дробной границе исполняет Original next Ground
callback и точный участок Source61 C8E58..C8EAC (все21 инструкции проверены).
Арифметика таймера в тесте не дублируется; допуски не ослаблялись.

Fresh163:24 settings —20/30Hz, камеры12/2,60/73/120/144/165/360FPS.
216 Source64/67 +192 Native constructor/idle/full-recovery schedules,
264 Source65/66 +144 Native off-grid command schedules.23 поля, точные
состояния/фазы/счётчики/количество реальных команд; A/B, target/D4, внешние
angle/speed writes, реальная привязка модели между Source-шагами.
Все прежние связанные ground/flight/damage/weapons/projectiles/world/camera
группы прошли.427 Original routines/regions; ASan/UBSan/LSan. Четыре fresh
player/model/loop/simulation отчёта сверены с generation receipt/source SHA.

Комната/модели синтетические; звук, lookup/release, sprite5 и неперенесённые
эффекты инструментированы. Model3 ctor/attachment/destructor исполняются;
настоящие уровни/визуал этим CPU-тестом не подтверждены. Пользователь
подтвердил запуск034 без вылетов, FPS/сцена/длительность не указаны.
Последний EXE034/v130 сохранён неизменным; нового EXE нет. main/HEAD,
донор и32 закреплённые зависимости проверены. Игра/окно/renderer/GPU
разработчицей не запускались. Source checkpoint163 сохранён.

Полный variable FPS остаётся в работе:49/63, смерти и другие действия,
active AI/платформы, сцены, сценарные камеры, катсцены, миниигры и2D.
Работа продолжается без ожидания playtests и без промежуточных EXE.

---

## Продолжениеv152 — здоровый урон32..48 и recovery31, без EXE

Подключены recovery31 clip10/stop/return и waiting46/47:20-count ожидание,
один kick15/yaw reversal/unpause, настоящий fall/landing, clip30 и rearm30.
Constructor period46 не получает ранний first decrement из extra render part.
Общая CC=1 запись сохраняется при подавлении повторного impulse; primary46
clip планируется с rate1, mirror с rate3. Высота с реальным gravity command
вычисляется от guarded Source baseline до Original collision solver, с точным
Original float endpoint. Столкновения/внешние команды/identity/run/time gaps
перебазируют clock. Эпсилонного прилипания и коррекции после solver нет.

Fresh152:414 Original функций,24 settings:20/30Hz, камеры12/2,
60/73/120/144/165/360FPS.48 Source recovery31 и48 waiting46/47,
336 Native real hit/health/rumble/full recovery schedules,144 lethal handoffs.
Все прежние связанные группы ground/flight/weapons/objects/cameras прошли.
Player/model/loop/simulation отчёты сверены с receipt и source SHA. Негативные
149..151 результаты сохранены; допуски сравнений не ослаблялись. Independent
Source46/31 выполняет Original player→world origin→camera, включая steering.

Комната/модели синтетические; звук, ресурсы/спрайты инструментированы.
E5C98/E5BF8 исполняются Original, E6968 — checked failed-allocation boundary:
настоящая жизнь/визуалFX и реальные уровни этой проверкой не подтверждены.
Пользователь подтвердил запуск034 без вылетов; уровень/FPS/длительность не
указаны. Последний EXE034/v130. Новый EXE на этот промежуточный шаг не создан.

Полный variable FPS остаётся в работе: прочие действия/смерти, active AI,
платформы, сцены, сценарные камеры, катсцены, миниигры и2D. Работа продолжается
без ожидания playtests; игра/окно/renderer/GPU разработчицей не запускались.

---

## Продолжениеv148 — связанный здоровый урон32..45/48, без EXE

Завершена CPU-проверка healthy damage32..45 и48 через настоящий удар, импульс,
падение, столкновения, клипы приземления/подъёма и полное восстановление.
Hazard40 сохраняет freeze/hide до count15 и один kick70 на16;43 один kick70.
Собственная pause/unpause команда не разрешает второй шаг счётчика в том же
Source периоде. External run/lifetime replacement по-прежнему сбрасывает guard. После real
collision private translation guard наблюдает actual coordinate, но при unchanged
velocity сохраняет Source candidate вместо накопления integer pushouts. Новый
velocity command после contact/external write rebases сразу; Source interval и
identity/run change также rebases. Итоговая позиция после solver не исправляется.
Контакт берётся из настоящего первого probe каждой фазы. Gravity/drive имеют
Source baseline, X/Z сохраняют precise sums до collision, rearm30 не получает
лишний decrement хвостом своего периода. Original20/30 сохраняет whole steps.

Fresh148:404 Original функции, оба20/30Hz, камеры12/2,60/73/120/144/165/360FPS,
96 hazard Source schedules,192 остальных healthy Source schedules,
288 Native real hit/health/rumble/full recovery schedules и144 lethal handoffs.
Все прежние связанные группы оружия/полёта/объектов/камер прошли. Четыре
fresh отчёта player/model/loop/simulation совпадают с receipt/source SHA.
Сохранён verified source checkpoint148. Ошибки146 own-pause и147 room-edge pushout документированы,
допуски проверок не ослаблены.

Комната/модели синтетические, звук/цвет/ресурсы инструментированы. F3/F8/FA
читаются реальными probes из triangle metadata. E59BC/E5894/E5B20/E5A24
исполняют Original с проверенным отказом allocation E6968; жизнь/визуалFX
и настоящие уровни этим не проверены. Пользователь подтвердил запуск034
без вылетов; уровень/FPS/длительность неизвестны. Последний EXE034/v130.

Полный variable FPS остаётся в работе: дальнейшие состояния персонажа,
смерти, активный AI/платформы, сценовые режимы, сценарные камеры, катсцены,
миниигры и2D. Новый EXE после крупного завершённого элемента. Игра/окно/
renderer/GPU разработчицей не запускались; продолжение без ожидания тестов.

---

## Рабочий кандидат147 — следующие опасные реакции40..45, без EXE

После verified checkpoint145 подключены healthy40..45: исходные freeze/hide
через counter15, kick70 на16 для40, one-shot70 для43, настоящий fall/landing,
collapse/rise и rearm30. Counter commands выполняются один раз за containing
Source период; drive/gravity/contact/model остаются непрерывными. Landing
использует fresh guarded probe, scalar/vertical baselines и precise X/Z sums145.

Новые проверки:96 Source hazard schedules и ещё96 Native floor-hit schedules
через real F3/F8 triangle metadata/C2978/health. E5B20/E5A24 и lookup/reset contacts
исполняют Original; E6968 checked failed allocation для assets39C/3E0. Настоящая
жизнь/визуалFX и color commands не проверены. Диагностика146 выявила own-pause run invalidation:40 count16 ошибочно
переходил в100 в том же Source периоде.147 записывает run после собственной
pause/unpause команды; диагностика ещё pending. Отчёты145 НЕ проверяют147.

Verified source145 и Windows034/v130 сохранены. Новый EXE пока не собирается.
Другие опасности/смерти/действия, active AI/platforms, сценовые варианты,
сценарные камеры, катсцены, миниигры и2D остаются в полном активном объёме.
Игра/окно/renderer/GPU не запускались разработчицей; работа продолжается.

---

## Продолжениеv145 — связанный здоровый урон и восстановление, без EXE

Ground32..39 и flight48 подключены: настоящий импульс, непрерывное падение и
столкновения, приземление, клипы collapse/rise, восстановление движения и rearm30.
Source contact берётся из настоящего C0C7C первого hurt callback каждого периода,
защищён state/actor/physical/model lifetime/run и рациональным временем. Новый
удар не наследует старый grounded contact. Clamp drive сохраняет Source endpoint
и kick/drift без коррекции позиции. Vertical gravity также вычисляется из
guarded Source baseline: даже длинное падение не суммирует округления substeps.
Horizontal X/Z сохраняют precise delta sums до collision; actual contact и
внешняя запись инвалидируют сумму, позиции после solver не корректируются;
новый cooldown30 не уменьшается собственным
Source хвостом. Original whole ticks и20/30FPS сохраняются.

Fresh CPU145: по48 Source schedules flight48/ground35..36/heavy37..39/air32..34;
192 Native real hit schedules в73ms со здоровьем, единственным rumble, импульсом
и полным восстановлением;72 lethal ground и72 lethal flight handoffs до второго
callback/движения/анимации. Вся прежняя связанная матрица оружия/полёта/камеры
прошла: оба dividers20/30Hz, камеры12/2,60/73/120/144/165/360FPS,384 Original
функции, ASan/UBSan/LSan. Четыре свежих отчёта player/model/loop/simulation
совпадают с receipt и текущими хэшами исходников; сохранён source checkpoint145.

Комната/модели синтетические, звук/цвет/ресурсы инструментированы. FA hazard
читается реальными probes из triangle metadata. E59BC/E5894 исполняются Original,
E6968 проверен с отказом allocation: настоящая жизнь/визуалFX не проверены.
Это CPU-проверки, не прохождение. Пользователь подтвердил запуск034 без вылетов;
его уровень/FPS/длительность не указаны. Последний EXE остаётся034/v130.

Полный variable FPS ещё в работе: следующие опасности/смертельные действия,
прочие состояния персонажа, активный AI, платформы, сценовые режимы, сценарные
камеры, катсцены, миниигры и2D. Новый EXE после крупного законченного элемента.
Игра/окно/renderer/GPU не запускались разработчицей; работа продолжается.

---

## Продолжениеv130 — связанный полёт, воздушная стрельба и передача управления

Подключены состояния20/27/57/58 при включённом полёте. Скорость, ориентация,
проекция и столкновения обновляются по native clock. Пара скорости/смещения
вычисляется из исходных команд; снимок защищён actor/physical/model lifetime/run,
точным вводом/параметрами/контактами и рациональным временем. Число случайных
решений и звуков полёта сохраняет исходный темп. Поворот360→0 сохраняет исходное
представление на границе команды, не создавая ранний скачок.

Воздушное оружие9/10 использует реальный нулевой выстрел и повтор с кадра18,
11/12 — заряд с кадра18. Снаряд получает только время после собственного рождения,
включая B между исходными тактами. Все прежние наземные действия оружия, остановка
и возобновление взаимодействия13, attachment snapshots и27 снарядов сохранены.
Урон переводит57 в исходное действие48; старый native проход прекращается до
следующего callback, движения и анимации.48 пока остаётся у Original.

Fresh CPU v130:96 полных Source flight schedules по24 endpoints,24 смены A/стика
в73/173/267ms,192 Source schedules воздушного оружия с полным pool и освобождением,
96 Native press/release,24 held-B schedules по4 выстрела,72 реальных damage handoffs.
356 исходных функций; оба делителя20/30Hz, камеры12/2,60/73/120/144/165/360FPS.
Проверки player/model/loop/simulation совпадают с receipt и текущими source hashes.
ASan/UBSan/LSan используются CPU-проверками. Модели/комната синтетические;
контакты взаимодействия, ресурсы/спрайты и звук инструментированы. Это не прохождение.

Windows test-034/v130 успешно скомпилирован и сохранён в общей testbuild папке.
EXE SHA256: ed0337e4fcf2c441eccf757bf9043d501257f8157c455a53cf9ca2e69161f23a.
Полный variable FPS НЕ завершён: остальные действия/режимы персонажа, активный AI,
платформы, сценарные камеры, катсцены, миниигры и2D требуют дальнейшей интеграции.
Игра/окно/renderer/GPU не запускались; Original20/30 и Manual60–360 сохранены.

---

## Продолжениеv128 — связанное оружие и pause/morph/attachment, без EXE

Player28/29 weapon1/13 подключены вместе с17/18,21/22,24/25 через реальные
Original commands. Weapon13 останавливает clip18, ждёт окончания interaction105,
снимает pause и возвращает исходный morph source. Source ctor/pool независимо
проверяет точку Native выстрела weapon1/2/4/5 по предыдущему action interval,
включая off-grid B. Снимок защищён epoch/actor/physical/model lifetime/run.

Bounce82 сохраняет непрерывные probes, Source-end surface phase commands,
немедленный actual actor contact и birth-relative начало подъёма после импульса.
Его model-scoped скорость вычисляется от начала Source такта, поэтому накопление
float округлений не откладывает точную границу50→10 на ещё один такт.
Пара скорости/смещения остаётся в той же непрерывной collision scope.
Interaction105 потребляет сохранённое окончание clip на проверенной retirement
границе, даже если raw end pulse уже исчез между Source концами.

Проверкиv128:864 Source moving actions до120 endpoints полного pool,
432 Native press/release schedules,192 независимых attachment comparisons,
48 held-B schedules по4 shots, прежние192 charge actions/96 Native charge inputs.
Original Source pause/busy/morph сравниваются на каждом endpoint для13.
Четыре fresh CPU отчёта совпадают с generation receipt и текущими source hashes;
ASAN/UBSAN, обоих dividers и60/73/120/144/165/360FPS. Синтетические ресурсы,
контакты/звук и комната не заменяют прохождение настоящих уровней.

Остаются special flight states20/27/57/58 и другие действия персонажа,
активные NPC/платформы, другие сцены, сценарные камеры, катсцены, миниигры и2D.
Полный variable FPS не завершён. Последний Windows EXE test-033/v89.
Игра/renderer/GPU не запускались. Следующий EXE после крупного связанного блока.

---

## Продолжениеv121 — движущееся оружие, ввод и повторная стрельба, без EXE

Связанный путь player24/25 теперь подключён вместе с21/22 и17/18. Оружие
2/3/4/5/6/7/8 использует реальные initial commands, контакты, дочерние эффекты
и полное освобождение ресурсов. Native нажатие/отпускание между Source тактами
задаёт фактический момент рождения. Мгновенный cue0 получает elapsed движение
только своей новой allocation lifetime; уже выполненный Source cue0 не повторяется.
Held-B weapon6 выполняет repeat18 на точной границе, разделяя event cursor
с ordinary callback: нет дополнительного выстрела на следующем Native кадре.

Проверкиv121:672 moving Source schedules до120 endpoints полного pool,
336 Native press/release schedules,48 held-B schedules по4 shots; прежние
192 charge actions и96 Native charge B schedules также прошли. Статическая
модель86 проверяется по actual trajectory, animated models по elapsed age.
326 Original функций исполняются в CPU closure. Свежие model/loop/simulation
отчёты совпадают с receipt и хэшами текущих исходников. ASAN/UBSAN прошли.
Синтетические ресурсы/комната и instrumented sound/AI не заменяют прохождение.

Остаются player27/28/29, charge20, Native attachment Source-position reference
для2/4/5, активные NPC/платформы, другие игровые режимы, катсцены, миниигры и2D.
Полный variable FPS ещё не завершён. Последний Windows EXE — test-033/v89.
Игра/renderer/GPU не запускались. Следующий EXE — после крупного связанного блока.

---

## Продолжениеv116 — связанные выстрелы и время от рождения, без EXE

Подключены player21/22 и реальные выстрелы оружия2/4/5 через общий dispatcher,
weapon constructor table и существующее Native обновление всех27 projectile
callbacks. Cue15 выполняется после продвижения модели на точной границе времени.
Начальная команда новых снарядов выполняется один раз; живые объекты не получают
повторного шага. Воздушный выстрел использует исходную позицию крепления до
движения персонажа. Поверхностные преобразования84/88 сохраняют Source endpoint,
а контакт с актором остаётся немедленным. Дочерний89 потребляет завершение клипа
после продвижения модели, включая клипы, заканчивающиеся между Source тактами.

Native нажатиеB между тактами теперь проверено для оружия11/12: shot и длительность
charge отсчитываются от фактического рождения, а не от общей сетки игры.
У projectile отдельный clock; пауза приостанавливает его возраст. Allocation token
модели отделяет повторное использование тех же адресов от прежнего объекта.
Общий clock персонажа/камеры/ввода сохраняется. Raw model end flags не меняют
семантику; отдельное observation позволяет дискретному owner увидеть пересечение
в конце своего такта. Исходный путь Original сохраняет20/30Hz.

Свежие CPU проверкиv116:192 charge action/Source B schedules,96 Native B@73ms
exact shot/birth-relative lifetime schedules и288 moving weapon schedules
(земля/воздух, клипы3/7, оба делителя,60/73/120/144/165/360FPS, две камеры).
Сравнивается весь pool на каждом из70 Source endpoints и полное освобождение
ресурсов; первая Original RAM совпадает. Прежние связанные проверки тоже прошли.
Model, общий loop и simulation отчёты сверены по receipt и текущим source hashes.
Ресурсы/комната синтетические; это не прохождение реального уровня.

Следом остаются weapon states24/25/27/28/29, charge20 и повторные/заряженные
выстрелы; Native B entry оружия2/4/5 тоже требует связанной проверки.
Active AI, катсцены, миниигры и все2D остаются в работе. Полный variable FPS
ещё не завершён. Windows EXE остаётся test-033/v89; новый — после крупного блока.

---

## Продолжениеv110 — игрок17/18 → charge11/12, без EXE

Настоящие наземные/воздушные действия игрока17/18 создают charge101/103 через
full player dispatcher и исходный weapon constructor table. Native owner
потребляет frame18 после движения и model advance. Интервал делится у frame18,
поэтому constructor вызывается ровно на исходном времени при произвольном FPS.
Pose acquisition игрока17/18 восстанавливает observation до Original counter.

192 joint CPU расписания: два оружия, земля/воздух, direct action init либо
реальная кнопкаB в Source entry с последующим удержанием;24 FPS/divider/camera
комбинации. Проверяется полная RAM первого Original прохода, единственный
constructor в том же времени, birth clock0, длительность charge8 исходных
тактов и полное освобождение parent/converted effect ресурсов. Общие animation,
loop и simulation проверки тоже свежиеv110. Синтетические room/model assets;
это не прохождение уровня. Native input edge внутри уже активного оружия
и остальные weapon player states ещё требуют подключённой проверки.

Все27 projectile callbacks уже имеют reviewed Native owner. Active AI,
катсцены, миниигры и все2D остаются в работе; полный variable FPS не завершён.
Windows EXE остаётся test-033/v89. Следующий EXE — после крупного связанного блока.

---

## Продолжениеv109 — interaction105, без EXE

Подключены все27 projectile callbacks scene1. Interaction105 выполняет
attachment и пространственное выдвижение на каждом Native update; удерживаемый
NPC получает текущую позицию. Reach использует реальные ключи Original через
три кадра анимации. Phase/seek/spawn/retirement остаются однократными командами
Source endpoints. Захват хранит actor/physics/model и allocation lifetime;
переиспользованный NPC не наследует старый захват. Source commands после AI
восстанавливают удержание в том же времени, не продвигая clock повторно.

120 joint CPU расписаний покрывают свободное действие, удержание, seek/spawn,
phase1→2 и реальную phase0 capture branch с инструментированными contact flags.
Сравниваются Original/v61 по полной RAM первого прохода и полям каждого такта;
54 промежуточных точки удержания за расписание проверяются по геометрии реальных
Original callbacks.48 live births включают полное освобождение parent/child,
сохранение commanded pose без dt до рождения и same-address NPC recreation.
Функции Original движения игрока участвуют в сравнении. Все четыре проверки
свежиеv109, без игры/GPU; Model component отдельно проверяет outside-clip seek.

Это CPU покрытие с синтетическими ресурсами/комнатой и paused NPC; реальные
weapon player states, active AI, катсцены, миниигры и все2D ещё в работе.
Общий variable FPS не завершён. Windows EXE остаётся test-033/v89; новый EXE
будет после крупного связанного блока, а не после отдельного callback.

---

## Продолжениеv107 — траектория самонаведения86, без EXE

Криволинейное движение86 вычисляется на каждом Native-интервале от исходного
состояния такта. Коррекция кривизны исчезает на Source endpoint, сохраняя
перемещение Original. Выбор цели и знак angular command используют исходный
наклон/курс и прогнозируемую конечную точку; текущие xyz после чистого запроса
восстанавливаются. Новый базовый наклон вводится плавно, без скачка на границе
Source такта. Clock движения привязан к allocation модели: pause gap, внешняя
команда, acquisition, partial heap reset и reset системы отменяют старую выборку.

384 CPU расписания покрывают два персонажа, нулевую и штатную скорость25/7,
обычную/почти выровненную цель, поворот через360 и отсутствие цели. Движущиеся
варианты проверяются6 Source тактов; stationary —102, включая конец tracking
window4..94. На каждом такте сравниваются8 полей с независимым Original/v61,
на четвертях — xyz по независимой кривой из реальных Original projections.
На каждом Native update ограничена скорость изменения наклона; реальные hit87,
полное удаление parent/child и освобождение ресурсов тоже проверены. Цели
статические; движущиеся AI и реальные уровни ещё не подтверждены.

Все четыре CPU checks свежиеv107, без игры/GPU. Native callbacks26 из27;
26 families имеют описанное CPU покрытие. Interaction105, старшие действия
игрока, active AI, катсцены, миниигры и все2D остаются в работе.
Windows EXE по-прежнему test-033/v89; новая сборка не делалась.

---

## Продолжениеv105 — charge101/103, общий birth boundary, без EXE

Подключены оба заряда: real constructor/resource lists, привязка к текущему
персонажу, дробный рост радиуса и opacity, phase34+3, one-shot conversion в
102/104 с rumble и полным retirement.48 acquired-Original расписаний сравнивают
все Source endpoints сv61 вместе с настоящим движением игрока. Ещё48 live-birth
расписаний вызывают реальный constructor во время Native player callback:
начальные команды выполняются один раз, model clock=0, phase21/radius10,
conversion через8 Source ticks от рождения, полное освобождение ресурсов.

Снимок projectile pool перенесён перед всеми producers, включая игрока: новый
actor не получает dt до рождения. Birth acquisition одного model сохраняет
scalar clocks остальных владельцев. Original/native-born opacity provenance
различается; компонент проверяет отсутствие overshoot и cache preservation.
Все четыре CPU checks свежиеv105: joint gameplay, model, loop, simulation.
Игра/GPU/Windows-сборка не запускались; EXE по-прежнему test-033/v89.
Активных projectile callbacks26 из27; полностью проверены25 families.86 ещё
требует moving curved trajectory;105 — frame-dependent reach/seek/NPC drag.
Старшие действия игрока, active AI, катсцены, миниигры и все2D не завершены.

---

## Продолжениеv102 — clocks самонаведения86, без EXE

Реальные ctor/callback86, выбор цели, oscillator36, tracking34, yaw1/1.5 и
pitch2 переведены на общий Native timeline. Дробная фаза short сохраняется
на время жизни модели; направление angular command фиксируется на текущий
исходный тик, а движение угла продолжается дробно.48 stationary tracking/
impact/retirement расписаний совпадают по счётчикам и базовым углам сv61;
joint прежних сценариев проходит. Model CPU дополнительно подтверждает12
oscillator и12 angular-command расписаний, wrap и partial heap invalidation.
Криволинейное движение86 с ненулевой скоростью ещё НЕ проверено; это частичная
интеграция86, не завершённая projectile family. Полностью проверены23 families,
активных Native callbacks24. Charge101/103 и interaction105 ещё не подключены.
Loop/simulation отдельный PASSv99; Windows/игра/GPU не запускались, EXE033/v89.

---

## Продолжениеv100 — отскок82 и переход82→83, без EXE

Подключён82: настоящие constructor, gravity, spatial probes, мгновенная смена
скорости при отскоке, создание83 и удаление родителя.24 расписания проверяют
Original-pass по всей RAM, исходный участок траектории по независимомуv61,
дальнейший полёт от реального момента отскока, единственный conversion/rumble,
нулевой clock ребёнка при рождении и полный четырёхстадийный lifetime83.
Joint player/world/projectile CPU check проходит вv100. Model/loop/simulation
последний отдельный PASSv99; повторно их на этом небольшом шаге не запускали.
Native ownership23 из27 projectile callbacks. Homing86, charge101/103 и
interaction105 остаются в работе. Игра/GPU не запускались; EXE test-033/v89.

---

## Продолжениеv99 — четырёхстадийный эффект83, без EXE

Подключён83 с четырьмя настоящими жизненными циклами модели и тремя bind
переходами. Смена стадии выполняется сразу после окончания клипа; новая модель
начинает с нулевого времени и не получает интервал до её создания.24 расписания
сравнивают положение, скорость, фазу, mask и alpha на каждом Source endpoint,
вплоть до полного удаления, с независимым Original/v61. Original-pass также
совпадает по полной RAM. Предыдущие цепочки, эффекты, игрок/камера и CPU loop/
model/simulation проверки проходят. Native ownership22 из27 projectile callbacks;
отскок82, homing86, charge101/103, interaction105 ещё в работе. Игра/GPU не
запускались. Последний EXE остаётся test-033/v89, Windows-сборка не делалась.

---

## Продолжениеv97 — падение, попадание и цепные эффекты, без EXE

Подключены84/88/89: действительная Native-гравитация, геометрические проверки,
попадание84→85 и цепочка88→89 с пятью звеньями.72 расписания проверяют восемь
Source endpoints по независимому Original/v61, единственное попадание/команду
отдачи, создание звеньев, отсутствие обновления до рождения и полное освобождение
моделей/physics/resource references. Таймер34 у89 меняется только на Source
endpoint; пространственное расширение34 у88 на10 при попадании не масштабируется.
Всего native owners21 из27 projectile callbacks, остальные семьи и весь FPS
продолжаются. CPU-проверки игрока/камеры, предыдущих эффектов, loop, simulation
и моделей проходят. Игра/GPU не запускались. Последний EXE test-033/v89.

---

## Продолжениеv96 — рост, вращение и затухание, без нового EXE

Добавлены типы85/87/90: размер и прозрачность меняются на каждом native
обновлении, вращение сохраняет25 градусов за исходный игровой шаг. Начало
затухания, возраст и удаление сохраняют исходное игровое время.72 расписания
при60/73/120/144/165/360FPS, обоих делителях и двух режимах камеры сравнивают
состояние на каждом Source-тикe с независимым нетронутым Original/v61.
Проверены168 цепочек полёта,216 конечных эффектов,912 common-world полных RAM
сравнений, игрок и камера.12 отдельных расписаний проверяют точные float
значения затухания и удаление, паузу, внешние записи и смену жизни модели.
Проверки только CPU, с ASan/UBSan/LSan; игра и GPU не запускались.
Native ownership сейчас18 из27 projectile callbacks; остальные семьи и
полный перевод игры ещё не завершены. Последний EXE остаётся test-033/v89;
новые Windows-сборки только после большого завершённого связанного блока.

---

## Продолжениеv93 — полёт и попадание, без нового EXE

Типы91/93/95/97/99/107 соединены с девятью конечными эффектами на общей шкале
времени. Проверены168 расписаний полёта/паузы/попадания/создания/освобождения,
включая переключение скорости107 с25 на30 через настоящее окончание клипа и
bind следующего. Original-проход сравнивается с нетронутымv61 по полной RAM.
Новорождённый эффект исключён из предшествующего интервала: в scratch-контроле
без защиты его clock сразу стал1.5 вместо0 при60FPS. После исправления время
жизни от рождения совпадает при60/73/120/144/165/360FPS и обоих делителях.
Настоящие constructors, модели, списки ресурсов и удаление выполняются; lookup/
release ресурсов и команда создания спрайтов наблюдательные на синтетических
данных. Это CPU-проверка, игра/GPU не запускались. Последний EXE test-033/v89.
Полный FPS и весь блок27 типов снарядов ещё в работе; далее рост/затухание,
цепочки, homing/charge/interaction, затем следующие части общей задачи.

---

## Рабочее продолжение 2026-09-30 — v93, эффекты завершения

В общий native timeline подключены девять конечных эффектов пула10..29:
типы92/94/96/98/100/102/104/106/108. Их реальные обработчики ждут завершения
клипа и один раз освобождают модель, актёр и physics. Original-пропуск разделён
по актёрам; остальные типы снарядов пока остаются исходными.

Передача существующей модели учитывает последнюю рассчитанную Original-позу.
Исходная функция сначала строит позу, затем прибавляет время; без этого учёта
эффект при смене владельца терял один исходный кадр жизни. Acquisition разрешён
только для проверенных общих объектов и этих эффектов, при свежем наблюдении
и совпадении клипа, каналов, скорости и времени. Действия игрока с кадровыми
событиями не включены в этот контракт.

CPU:216 времён жизни/удалений эффектов сравнены с нетронутым Original801D10D8,
912 полных RAM-сравнений общих объектов,24 расписания движения и48 разворота,
обе исходные частоты и60/73/120/144/165/360FPS. Model и общий loop прошли;
освобождение и контекст проверены, игра/GPU не запускались. Ресурсы и комната
синтетические, это не пользовательское прохождение. test-033/v89 остаётся
последним EXE; v93 ещё не компилировалась для Windows. Полный FPS не завершён:
полёт/попадания остальных снарядов, действия игрока17+, AI, катсцены, миниигры
и оставшееся2D продолжаются. EXE только после крупного связанного блока.

---

## Рабочее продолжение 2026-09-30 — v92, нового EXE нет

Последняя Windows-сборка остаётся test-033/v89; пользователь подтвердил, что
дрожание в городе ушло. Новый порядок: EXE только после завершения крупного
связанного блока, без сборок на каждую промежуточную правку.

В рабочем main допуск игрока расширен до состояний1..16. Поворот назад,
торможение, завершение клипа и переходы12→13/15→16 согласованы с исходным временем.
Общий диспетчер мира801E8BDC теперь делит владение по актёрам: 38 проверенных
типов с пустым обработчиком801E5D84 получают настоящее native движение,
прозрачность, модельные часы и исходную частоту мигания/спрайтов. Остальной AI
пока обновляется исходным путём. Native-проход сохраняет контактные списки.
Перед командами исходного world update вычисляется прошедший интервал, поэтому
новая скорость и появление/исчезновение объекта не применяются задним числом.
Границы клипов учитывают только действительно обновляемые модели и их текущую
скорость, включая новые клипы и исключая остановленные объекты.

CPU:24 совместных расписания игрока/мира и48 разворота/торможения; обе исходные
частоты,60/73/120/144/165/360FPS, обычная камера12 и городская2. Проверены реальные
модельные/flipbook функции, смена скорости, появление/исчезновение, исходные
счётчики, контактные списки, CPU-контекст и освобождение моделей. Original RAM
совпадает с исходным801E8BDC для всех38 типов (912 сравнений). Model/loop/time
проверки тоже прошли; ASan/UBSan/LSan там, где предусмотрены проверками.
Синтетическая комната/ресурсы и наблюдательные sound/AI endpoints ограничивают
этот результат: это не прохождение и не подтверждение всей логики NPC.

Полный variable FPS ещё НЕ завершён. Далее — снаряды/эффекты пула10..29,
действия игрока17+, активный AI всех overlay, катсцены, миниигры и остальное2D.
Игра/окно/renderer/GPU не запускались. Рабочая v92 проверена на CPU; её Windows
компиляция ещё не выполнялась. Старый windows-build-check.json относится к033.

---

## Продолжение 2026-09-30 — test-033 (генерация v89)

Исправлена пропущенная публикация точки отсчёта отображения мира после native
движения игрока. В test-032 она оставалась в исходном world update20/30Hz,
а персонаж и относительная камера уже обновлялись чаще. Пользователь сообщил
о дрожании персонажа и3D-домов в городе, сильнее вдали от камеры. Его runtime.log
1790765643377 подтвердил test-032, Manual60 и городской обработчик801D7294.
CPU-проверка до исправления воспроизвела разность2.25 единицы между позицией
игрока и render origin на13-м кадре60FPS. Теперь после движения вызывается
оригинальный801BBDDC до камеры; координаты общего кадра согласованы каждый шаг.
Пользователь подтвердил: в test-033 дрожание ушло. Это подтверждение его
проверки города; игру/renderer/GPU при разработке не запускали.

Допуск player preview расширен на состояния1..10: ходьба/бег, прыжок, падение,
приземление. Модельные команды теперь проверяются в той же CPU-цепочке, что
ввод, полный player dispatcher, поиск пола, drive, контакт и камера, вместо
подмены model endpoints.182 оригинальные функции,24 расписания: оба исходных
делителя,60/73/120/144/165/360FPS, камеры12 и городская2. Удержание A даёт один
прыжок и одно приземление; позы свободного банка восстанавливают игровые кости.
Синтетические модели/комната и наблюдательные sound/sprite endpoints остаются
границами проверки. Это не прохождение уровня. Общий loop/barrier тоже проверен
после изменения; остальные актуальные CPU-отчёты сохранены без лишнего повторения.

Остальные действия, NPC/объекты, катсцены, миниигры и меню/2D ещё требуют общего
перевода. Полная цель variable FPS НЕ завершена. Original20/30FPS и существующий
Manual60–360 сохранены; RT64 interpolation нет. Название policy в старом логе
scene1-ground-preview оставлено историческим; реальный допуск определяется кодом.

---

## Продолжение 2026-09-30 — v89 / test-032

Подключён полный обработчик наземного персонажа в состояниях1..5: свежий ввод,
решения состояний, скорость, столкновения, post-contact, часы моделей и копирование
трансформации спутника. Старый VI больше не выполняет этого игрока второй раз.
Общий VI продолжает обслуживать контроллеры, звук и ещё не переведённый мир.

Новый ввод хранит время VI-образцов и делит native-интервал на их границах.
Короткие нажатия расходуются один раз; при возврате в Original потреблённые
нажатия не повторяются, непотреблённые сохраняются. Именно CPU-проверка полного
обработчика выявила ошибку применения нового стика задним числом: при73FPS
траектория отличалась от60FPS на6.08 единицы. Очередь устранила расхождение.

Счётчики неуязвимости/максимальной скорости/бездействия/паузы действия и звуки
шагов привязаны к исходному игровому времени. Повторная установка неизменного
флага паузы/конца клипа не начинает часы заново. Native-поза вычисляется по
актуальному времени клипа в свободном банке с восстановлением игровых костей.

Смена сцены отменяет остаток накопленного интервала, включая вложенный вызов
камеры. Барьер ресурсов сохраняет очередь посторонних сообщений и ждёт обе
кадровые банки, позднее завершение SP и HostRead.

CPU-проверки: полный наземный обработчик с реальным поиском пола/объектов,
ввод, анимации, совместная физика/камера и общий loop прошли. Для наземного
обработчика и ввода проверены20/30Hz и60/73/120/144/165/360FPS. Исходные
анимационные кривые проверены отдельно; звук/model/sprite endpoints в полном
сценарии инструментированы. Синтетическая комната не заменяет прохождение.

Полный variable FPS всей игры НЕ завершён: остальные действия, NPC/объекты,
катсцены, миниигры и меню/2D ещё требуют подключения и проверки. При выходе
из допуска возвращается Original. Процесс разработки продолжается после EXE.
Игра, окно, renderer и GPU локально не запускались. Новых настроек и RT64
interpolation нет; ROM, port_*.json и saves при упаковке сохраняются.

---

История предыдущих этапов (ниже):

> Актуализация v76/test-029: связанные gravity/displacement и общий process wait/activation timing проверены и собраны. Живой Manual всё ещё scene34 image/white fade; полный native gameplay НЕ готов. См. native-simulation.md. Работа продолжается после сохранения EXE.

# Нативная переменная частота: начало реализации

2026-09-29. Продолжение `framerate-analysis.md`, после разрешения реализации.
Новая цель — настоящие игровые кадры без интерполяции RT64. Пользователь уточнил:
для испытаний использовать существующий ползунок **Manual**, а **Original** должен
сохранять исходный игровой цикл. Новых переключателей и отдельных файлов настроек
не требуется. Это документ о незавершённой реализации, не о готовом HFR-режиме.

## Актуальное состояние после подключения lifecycle

Есть native message bridge, отдельный scheduler pump и explicit present с учётом
завершения scheduler/game/renderer. Windows/Linux собраны; component checks пройдены.
Подробности: [native-presentation.md](native-presentation.md).
**Native Manual ещё не активен**. Теперь подключены producer timer и протокол
входа/выхода с gate настоящего gfx worker: [native-frame-session.md](native-frame-session.md).
Остались игровой scene adapter, его legacy boundary и перевод первой 2D-сцены. Последняя переданная сборка — test-024.
Ниже сохранены результаты последовательных этапов; поздние разделы уточняют ранние.

## Что реализовано и проверено

`src/native_frame_time.*` — изолированный компонент времени, фейдов и анимации:

- ManualClock рассчитывает сроки кадров по выбранному целому значению 60–360 из
  существующего диапазона Manual. Частота не привязана к 60, 120 или частоте монитора.
  Перенос целочисленного остатка исключает накопление ошибки от округления периода.
- FrameStep содержит фактически прошедшее время. Изменение ползунка не теряет время
  после предыдущего принятого кадра. После задержки выдаётся одно актуальное обновление
  со всем прошедшим временем, а не пачка устаревших кадров. Физике ещё потребуются
  ограниченные подшаги: передавать ей весь большой интервал напрямую нельзя.
- Original и Display не активируют этот компонент. Это только политика нового
  компонента; существующий Display в живой сборке пока работает прежним способом.
- SceneClock различает подтверждённые делители 2 и 3, то есть исходные 30/20 тиков.
  Смена сцены требует явного создания нового часово́го контекста. Неизвестный делитель
  отклоняется, а не молча считается равным 3.
- Fade вычисляет альфу по времени. Значения на исходных границах тиков и момент
  завершения совпадают с оригиналом, включая последний неполный шаг, step=0 и
  уже достигнутую границу. Между ними появляются дополнительные значения альфы.
  Повторная команда того же фейда должна явно создавать новый экземпляр.
- Animation считает позицию клипа от накопленного времени, без суммирования float
  по одному маленькому шагу каждого кадра. Поддержаны проверенные pause, forward clamp,
  прямое/обратное проигрывание, несколько оборотов при задержке. Завершения отдельно
  сохраняются до `consume_completions()`, чтобы их не затёр следующий native update.
  Переход/seek/изменение скорости/повторное использование модели требуют нового
  экземпляра, а не попытки распознать это по равенству значений.

Состояние каждого объекта компонента имеет постоянный размер, динамических контейнеров
или фоновых потоков здесь нет. Реестр игровых моделей ещё не реализован; это не
утверждение об отсутствии утечек в будущей интеграции или во всей игре.

На первоначальном этапе компонент времени был включён только в сборочную цель.
В v62–v64 добавлены неактивные без native consumer hooks loop/scheduler/present;
они не меняют исходный игровой путь. Привязки таймера и сцены пока нет. Manual в последнем
переданном test-024 всё ещё означает прежний вывод через RT64. Новый EXE для проверки
HFR не передавался. Нельзя объявлять новый режим включённым по наличию этих классов.

## Проверки без запуска игры

`tools/check_native_frame_time.py --generated build-local/startup-v61` проверяет
receipt всех исходников генерации и собирает два оригинальных resident-тела отдельно
на синтетической RDRAM. Извлечённый код остаётся в игнорируемом build-local.

Проверены 60, 61, 73, 90, 120, 144, 165, 240, 359, 360 FPS; отсутствие ранних кадров,
сохранение времени 20/30 Hz за минуту, смена режима/ползунка, задержка и явный resume.
Для fade выполнено 655360 сравнений с **оригинальной функцией**, а не копией новой
формулы; отдельно проверены дробные обновления и завершение на разных FPS.
Для Animation выполнено 384 сравнения с оригиналом в подтверждённом диапазоне
одношагового обновления; отдельно — многократный wrap, сохранение событий и одинаковая
позиция после одинакового времени на разных FPS. ASan/LSan; UBSan для нового C++.
Оригинальный generated C использует MIPS signed shifts, поэтому UBSan на нём отключён.

Собраны desktop-цели Linux и Windows. Это проверка компиляции, не плавности, физики
или игровой совместимости. Рендер, игровое окно и звук агент не запускал. Android
сборкой этой итерации не проверен. Машинные отчёты:
`research/native-frame-time-check.json`, `research/native-frame-time-build-check.json`.

## Новые подтверждённые причины, которые нельзя пропустить

**Анимация уже допускает дробное время, но её события — нет.**
8003948C получает float increment в a1; время модели лежит по +64, активная анимация
по +60, данные по +18, длительность по +4 данных. По +7C bit3 останавливает обновление,
bit0 ограничивает конец при движении вперёд, bit1 сигнализирует выход за границу.
Следующее обычное обновление сбрасывает bit1. Это воспроизведено оригинальным телом.
Большой шаг оригинал оборачивает только один раз: 0+25 при длине10 даёт15. Поэтому
одного умножения аргумента на delta time недостаточно ни для событий, ни для задержек.
Сохранение событий реализовано в компоненте; конкретные игровые потребители ещё
нужно подключить, поэтому исправление в самой игре пока не заявляется.

Найденный прямой вызов 8003948C расположен в обёртке **8002A1B8**. Она сначала вызывает
800393C4 (вычисление каналов/позы, включая morph), затем продвигает время. Нельзя
незаметно поменять этот порядок: получится иной фазовый сдвиг анимации. Модель берётся
из массива по object+74+index*4. Seek-обёртка8002A41C сначала округляет float к integer.
Инвентарь прямых вызовов не исключает косвенные вызовы и обращения через данные.

**Исходный scheduler связывает готовность framebuffer с VI.**
80026CEC возвращает ненулевой аргумент только при равенстве current/next framebuffer.
80026D40 подтверждает завершение задачи после снятия ресурсных битов, затем выполняет
swap для graphics flags60. 80026A28 обрабатывает командную очередь на retrace и
рассылает события клиентам, включая звук. Увеличивать частоту общего VI-потока ради
Manual нельзя: это затронет AI, исходные таймеры и сроки освобождения памяти.
Нужен отдельный графический источник событий с сохранением реальных SP/DP/RT64
подтверждений. Снимать gate вслепую или подделывать task completion нельзя.

**Phase1 не доказана как чистое рисование.** В основном блоке scene1 проверена пара
801BB730/801BB848, устанавливаемая 801BB924. Ветви gameplay — 801BB080/801BB334.
В update есть счётчики demo и времени игры; render вызывает дополнительные callbacks.
Аудит всех их побочных эффектов ещё не завершён. Произвольный повтор phase1 не
считается ни безопасным, ни настоящим ускорением движения.

Адреса и ROM-хэши кандидатов воспроизводит `tools/analyze_native_frame_time.py --rom ...`.
Результат — `research/native-frame-time-analysis.json`. ROM/байты/ресурсы в отчёт
не записываются. Для реализации использовались оригинальный ROM и pinned исходники;
новых предположений о совместимости с первой игрой нет.

## Следующая рабочая граница

1. Подключить графические deadlines к игровой очереди отдельным типом события;
   реальные VI/AI, retirement ticks и Original оставить на исходном пути. Проверить
   два банка, завершения, scene transition, остановку и resume без игры на fake endpoints.
2. Завершить аудит phase1 выбранной 2D-сцены и одного gameplay-состояния. Подключить
   реальное обновление fade/animation и затем непрерывное движение/камеру/физику;
   сохранить длительности дискретных ожиданий и однократность событий/ввода.
3. Только после работающего полного пути включить Manual в native-режим, отключить
   интерполяцию RT64 для него, согласовать VSync с native target и вывести счётчики
   реальных update/build/present в диагностический журнал. Не оставлять подпись,
   обещающую native FPS, пока настройки ещё управляют старым RT64-путём.
4. Сохранять каждый собранный Windows EXE в testbuild и продолжать общий перевод
   без ожидания запуска пользователем (уточнение 2026-09-29).

Открыты также переключение Manual↔Original на безопасной границе (без пропуска
накопленных событий), владение native состоянием при смене/освобождении моделей
и проверка побочных эффектов CPU morph. Полноценный произвольный FPS всей игры
этой итерацией не завершён.

## Продолжение: протокол кадров и настоящие2D-команды
Добавлен src/native_frame_scheduler.*: два банка, независимые подтверждения,
поколения запросов и mailbox для32-битного сообщения. Сквозная проверка строит
новые оригинальные2D display lists для native fade60/73/144/165/360FPS.
Живой адаптер очередей ещё не подключён; подробности и границы проверки —
docs/native-frame-scheduler.md. Original и test-024 не меняются.


## Продолжение: подключён приём кадрового сообщения

Генерация v62 включает hook настоящего игрового message loop и проверяемый sender
в runtime. Отдельные VI и native события, переполнение очереди, старые токены,
регистры и сброс проверены на оригинальном loop и реальной очереди с synthetic RAM.
Подробности и ограничения: [native-frame-bridge.md](native-frame-bridge.md).
Native Manual ещё не активирован; пользовательский test-024 не заменён.


## Продолжение: запуск задач без дополнительных VI

Генерация v63 устанавливает native-only wake после публикации задачи и отдельный
вход в исходный scheduler без счётчиков/рассылки VI. Проверены реальная очередь,
приоритет audio, yield, SP/DP и full queue. См. [native-graphics-pump.md](native-graphics-pump.md).
Показ framebuffer и первая native-сцена ещё не подключены; Original/test-024 прежние.
Уточнено: донорский notify_cpu_framebuffer_write третьей игрой сейчас не вызывается.


## Продолжение: исправлена поздняя граница SP/DP, проверено рисование scene34

v65 переносит retirement за последние обращения SP/DP callers к задаче;
зарегистрированные type2 больше не теряются на паузе. Прежняя точка v64 была
слишком ранней; отрицательный стенд это воспроизводит. Подробности:
[native-presentation.md](native-presentation.md).

Реальная image-ветвь draw801C012C проверена вместе с её вызовами: новые2D-команды
и elapsed fade на60/73/144/165/360FPS без изменения остальной гостевой памяти в
проверенных состояниях. Это не scene adapter: игровая активация, Original/native
handoff и граница выхода перед transition остаются следующим шагом.
Контроллерный текст, общий build и update этим стендом не покрываются.
См. [native-scene34.md](native-scene34.md). Test-024 остаётся последней выданной сборкой.

## Критерий полного результата — уточнение пользователя

Нужен единый законченный режим Manual для всей игры: движение/физика/столкновения,
камера, model/sprite animation,2D/меню, таймеры/скрипты/события, все игровые режимы
и миниигры. Original сохраняет исходное поведение. Общие clocks/frame ownership
или одна работающая заставка НЕ являются выполнением этого требования.
По последнему уточнению пользователя каждый собранный Windows EXE сохраняется
в testbuild с честным описанием coverage, без просьбы запуска и ожидания. Отсутствие
вылета test-026 подтверждено пользователем, но это не подтверждение полного FPS.
Результат нельзя объявить полным при скрытом Original fallback отдельных сцен.

159 pending:158 compiled but ASan caught missing actor B0 mesh table in the
new64 reference. Fixture now supplies a distinct mesh table and executes actual
model3 constructor/attachment/destructor. Native acquired initial character1
auxiliary model is genuinely recreated after independent reference release.
Sprite5 remains instrumented; no real visual verdict. No production changes.

159 focused reached67 recovery:60FPS/divider2 scenario5 tick16 clip50 vs51,
all21 other fields matched.160 independent ceil remainder now executes actual
Ground first callback C3300 after67 recovery: C3124 changes clip2->0 and retains
even-frame seek. No production/tolerance change. Focused/full pending.

160 focused PASS:168 Source64/67 +264 Source65/66 +144 Native65/66;24settings,
426 Original functions and ASan/UBSan/LSan. Formal152 last complete checkpoint.
161 adds48 Source real low-health idle entry scenarios (counter298->299->300),
192 Native actual constructor73ms/character model3 lifetime/cancel/surface exit
and real idle entry schedules. Complete recovery and23 gameplay/count fields
checked. No production change; generation/focused/full pending.

161 strict negative:60FPS/divider2 idle Source tick1 clip~0 vs3, count0 vs300.
Fixture low health enters65 (Original branch), healthy enters64;162 corrects
the new idle cases to health10 and moves the actual idle increment/reset region
to once before the first callback in each Source period. Own clip/state change
does not reopen it. Independent ceil reference extracts actual Original Source61
C8E58..C8EAC region with complete instruction-address proof; no copied arithmetic.
216 Source +192 new Native special checks; fresh generation/focused/full pending.

162 passed new Source idle cases and reached Native mode7: synthetic immediate
return wrongly carried F1 into Ground, whose actual door callback needs scene
door data.163 applies F1 only to67 waiting modes5/6; immediate argument1 uses
ordinary floor. Fixture/report-only change. Fresh focused/full pending.

### Test-036 — передан завершённый городской блок

Windows EXE собран из matched generation193 и сохранён в
`D:/Games/DoraRecomp3/testbuild/Dora64-3-test-036.exe`.
SHA256 `9e9f6b1e5ab2787dac12a8c2e35118a7b1d4b5e199977998c96871693cafa99d`.
Семь текущих CPU отчётов, компилируемые inputs,32 dependency pins и donor
проверены. Все прежние файлы общей папки, ROM, настройки, saves и DLL
сохранены; архив source181/test035 создан перед обновлением Windows stage.
Игра/окно/GPU не запускались. Появление EXE не завершает полный variable FPS;
продолжается изолированный разбор slot09 NPC24..28 и остальных систем.

### NPC B regression — исправление порядка контакта перед оружием (195)

Пользователь: начиная с test032, test036 Manual60 использует оружие у всех
NPC города/других уровней; Original20 запускает диалог. На прежнем193 CPU
oracle воспроизвёл Original contact→NPC2/player66/busy1 против Native→NPC1/
weapon28/busy0. Native читала pressed B раньше общего collision dispatcher;
DD510 использует selected-controller801AC69C, которого нет в обычной
публикации NativeInputScope. После weapon28/29 C0624 уже запрещает talk.

На свежем B перед Native player callback добавлены Original eligibility,
точная collider geometry, порядок actor slots30..229, attachment parent,
реальный contact dispatcher и временная публикация selected B. DD17C
продолжает поиск только при table flag1; NPC flag0 выбирает первый контакт.
Двух-NPC отрицательный тест обнаружил неверное направление раннего кандидата;
это исправлено до main. Non-talk contact damage/pickups сохраняют Source
ownership; функция не запускает повторно общий AI/collision tick.

Изолированный ASan/UBSan/LSan PASS:3192 сценария,12 talk kinds, linked19,
Original20/30,Native60/73/120/144/165/360, близко/далеко, stale/fresh contact,
inactive/hidden/state/cooldown, последний world slot, два NPC/препятствие,
удержание B с одним initializer. Реальные Original callbacks работают
на synthetic geometry/assets; actual model creation/bind/free выполняются.
SoundF6 проверен отдельно от instrumented non-talk damage sound; dialogue
audio queue остаётся checked failure boundary. Eleven new semantic aliases,
twelve register scopes:654 descriptions/319 aliases/414 scopes; exact canonical
round trip/C tokens/C17/ABI PASS. Постоянный тест добавлен в player checker.
Общая проверка main195 ещё должна подтвердить остальные системы вместе.
Игра/renderer/GPU не запускались. Полный variable FPS остаётся незавершённым.

### Test-037 — исправлен порядок разговора и оружия

Windows EXE из согласованной generation195 сохранён в
`D:/Games/DoraRecomp3/testbuild/Dora64-3-test-037.exe`.
SHA256 `9be2f8a2d59d086fd5065aaa3e8456697222e213a3011f432b7d1c421e11f255`.
Регрессия test032–036: Native player callback получал свежий B раньше
Original contact selector/DD8FC, а selected-controller latch801AC69C
не получал этот B. Оружие переводило игрока в state28/29, после чего
Original C0624 отклонял разговор. Теперь свежий B сначала проходит
настоящую геометрию/порядок pool/parent и Original dialogue dispatcher,
а scoped selected-controller latch восстанавливается после команды.

Семь согласованных CPU отчётов generation195 PASS, включая прежние
движение, оружие, урон, модели и городские NPC, вместе с3192 новыми
сценариями NPC B. Дополнительный независимый прогон этих3192 сценариев
с настоящим player pool slot1 также PASS; он проверяет reciprocal
contact reset/categories и не меняет основной стенд. Его точные исходники,
Original object, исполняемый CPU стенд и hashes сохранены в
`build-local/npc-action-actual-pool-check/receipt.json`.
654 описания/319 смысловых имён/414 register scopes сохраняют ABI и
ctx register storage; документы с соответствиями обновлены.

32 dependency pins и donor неизменны. Прежние EXE, ROM, настройки,
saves и общие DLL сохранены; Windows source193/test036 архивирован.
Игра/окно/GPU не запускались. Реальные collider assets, UI/input/audio
и прохождение требуют проверки в игре пользователем.
Полный variable FPS остаётся в работе; slot09 NPC24..28 пока отдельный
кандидат, прочие NPC/уровни/платформы/сценарные камеры/миниигры/2D не завершены.

### Test-037 — пользователь подтвердил исправление диалогов

После передачи test037 пользователь сообщил: «диалоги норм теперь, срабатывают».
Результат и SHA EXE записаны отдельно в research/windows-user-test-037.json;
неизменный compile/package receipt сохраняет исходную проверку сборки.
Это подтверждение срабатывания разговора в игре; пользователь не уточнял
повторно набор NPC, уровни и FPS. Полный variable FPS остаётся незавершённым.

### Slot09 — проверены новые контакт/возврат/yaw кандидата198b, общий прогон в работе

После подтверждения test037 продолжен крупный блок NPC24..28 ROM1398A0.
Кандидат196a совместно проверяет idle/patrol/wait с рабочим NPC B fix195;
его полный общий player/world прогон ещё выполняется, main пока195.
Отдельный198b добавляет Native dialogue state2 для27/28: реальное Original
предложение один раз за исходный период, фактический промежуточный yaw
actor/physics и Original clips/counters/commands/return. Первые8 settings
PASS:336 настоящих constructors,1072 Source,240 Native edges,208 rejects,
7344 frozen189 полных RAM/context/heap/resource/command/model сравнений и
1440 промежуточных yaw samples. Закрытие, immediate return, оба signed
направления и wrap0/360 проверены. Настоящие mode4 contact producers27/28
проверяют все9 descriptor/quest ветвей с последующим закрытием/возвратом.
Старые3192 common NPC B сценария также PASS. Шесть компонентов198b PASS;
полный общий прогон198b запущен, завершение Native блока пока не объявляется.

Дополнительно1320 common collision/B сценариев для всех пяти новых kinds
24/25/26/27/28, Original20/30 и Native60/73/120/144/165/360, PASS. Проверены
first-contact/two NPC, non-talk obstacle, far/stale/inactive/hidden/state/
cooldown, последний pool slot, held B, context и selected-controller
restoration. Используются настоящие данные ROM1398A0 и Original callbacks;
для25 выбран sceneA1, для26 sceneAC. Geometry/model assets synthetic;
это отдельная проверка приоритета B, Native producer/body continuity27/28
имеет отдельную постоянную mode4 проверку. Две ранние компиляции этого
дополнительного стенда отклонились из-за отсутствия/повторного подключения
data header; порядок исправлен, прежние отрицательные стенды сохранены.
Игровой runtime и входы выполняющихся общих проверок не изменялись.

Каталог отдельного194m содержит690 описаний/355 aliases/442 scopes;
96 C units сохранили preprocessed tokens, ABI/register storage и C17 syntax.
Добавлены имена private dialogues24/25/26 и связанных Source pickup
обработчиков. D1498 описан буквально как публикация команды1 в802181DC.
Source-only анализ spawn descriptors:801FC818→kind118 (resources692/693),
801FC894→kind123 (712/713),801FC904→kind127 (710/711); точные данные scene1
совпадают с ROM. Их общая84-byte kind table и callbacks проверены; ранняя
экстраполяция частной NPC table801FAB48 за доказанный диапазон отброшена.
Это предметы, Native владение их движением/подбором пока не добавлено.

Далее: перенести196a после полного PASS, проверить все компоненты main;
после общего PASS198b подключить27/28 dialogue. Оставшиеся24/25 dialogue,
26 scene/quest/actual spawn+retirement и24/28 scripted states требуют своих
связанных проверок. Новая Windows-сборка на каждую правку не создаётся;
test037 сохранён. Полный variable FPS для остальных NPC/уровней/platforms/
script cameras/minigames/2D продолжается. Игра/renderer/GPU не запускались.

### Slot09 idle/patrol/wait — подключён проверенный общий блок (197)

Изолированный196a прошёл весь прежний player/world/NPC B прогон совместно
с NPC24..28 ROM1398A0, плюс шесть компонентов. Проверены1008 настоящих
constructors/288 skipped spawns,2400 Source/720 Native/720 rejection
сценариев и16320 frozen189 полных сравнений RAM/context/heap/resources/
commands/model count на24 settings. Реальные данные slot09 загружаются
только на время соответствующего блока; перекрываемые городские данные
восстанавливаются до городских проверок.3192 NPC B сценария сохранены.
Дополнительно64 перехода kind27 wait→dialogue проверили once-only,
overlay/table/physics/model/state/kind/reset rejection с сохранением
context и resources.

Исходники, generation и отчёты196a архивированы перед переносом. В main197
runtime/fixture/checker идентичны проверенному кандидату;96 canonical C
units, frontier/hooks/overlay table совпадают. Полный player report
сохраняет явное происхождение196a и доказательство переноса. Повторный
проверочный запуск всех моделей/циклов/времени/overlay/имён в main197
будет выполнен после переноса. Дополнительные25 имён и21 register scopes
из194m имеют отдельный token/ABI/syntax PASS; всего690/355/442. Source-only
pickup callbacks названы по точным внутренним ROM table именам; Native
владение предметами не добавлено.

Native владение:24 states1/2/4,25/26 states1/100/101,27 states1/100,
28 state1. Диалоги/event bodies пока сохраняют Source; отдельно198
проверяет расширение27/28 dialogue2, в main это расширение не включено.
Сохранённый test037 и подтверждённое исправление NPC B принадлежат
generation195. Новая Windows-сборка для этого промежуточного переноса
не создаётся. Полный variable FPS ещё не завершён; реальный запуск
игры/рендерера/GPU не выполнялся.

### Main197 — проверенный slot09 idle/patrol/wait и документация сохранены

Все семь текущих отчётов согласованы с generation197 и исходниками main.
Полный player/world report явно сохраняет происхождение выполненного196a:
runtime/fixture/checker byte-identical,96 canonical C units и frontier/hooks/
overlay table совпали после переноса. Имена/модели/simulation/frame-time/
loop/overlay заново проверены непосредственно в main197.690 descriptions/
355 aliases/442 scopes, C tokens/C17/ABI/register storage PASS; docs maps
обновлены. Main/HEAD,32 dependency pins, donor и прежний packaged034 EXE
прошли отдельную проверку сохранности. Main197 архивирован как следующий
проверенный CPU checkpoint; test037/Windows stage195 остаются прежними.
Кандидат198b выполняет общий прогон нового dialogue27/28; остальные NPC
states, предметы и полный variable FPS продолжаются. Игра/GPU не запускались.

### Slot09 dialogue27/28 — проверенный общий блок подключён (200)

Полный изолированный198b прошёл весь player/world/NPC B прогон совместно
с новыми dialogue2 у kinds27/28: фактические contact producers, девять
quest-dependent dialogue descriptors, close/return и shortest signed yaw
из оригинальной функции. Проверены24 settings,1008 actual constructors,
3216 Source schedules,720 Native edge schedules,624 rejections,22032
frozen189 full-RAM/context/heap/resources/commands/model-count comparisons.
Промежуточный yaw сравнивается с endpoints настоящего Original, включая
положительное/отрицательное направление и wrap. Старые3192 common NPC B
сохранены. Отдельный real-contact стенд пяти новых kinds24..28 прошёл1320
Original20/30 /Native60..360 action-priority/obstacle/stale/held-B случаев.

Перед переносом архивированы198b исходники/generation/reports и receipt
дополнительных1320 действий. В main200 runtime/fixture/checker идентичны
прогону198b;96 canonical C units и raw frontier/hooks/overlay table совпали.
Полный player report явно указывает origin198b. Дополнительные semantic
names main197 сохранены:690 descriptions/355 aliases/442 scopes. Шесть
компонентов main200 будут заново проверены непосредственно после переноса.

Native ownership теперь включает27/28 dialogue2. Остальные Source-only
сюжетные ветви24/25/26, spawned pickups и весь variable FPS продолжаются.
Source-only199 отдельно исследует sceneFD request/quest5B clear/set;
его новые сценарии пока не включены в основной fixture. Test037 и
Windows stage195 не менялись; пользователь подтвердил срабатывание
диалогов. Игра/рендерер/GPU не запускались, новый Windows build не создан.

### Main200 — dialogue27/28 проверен после переноса и сохранён

Все семь текущих отчётов согласованы с generation200 и исходниками main.
Полный player/world run имеет явное происхождение198b с проверкой всех
input hashes/96 canonical C units/raw frontier/hooks/overlay table.Фактический общий run дал11360 slot09 intermediate yaw samples.
Шесть компонентов заново выполнены в main200: models/simulation/frame-time/
loop/overlays/names PASS.690 descriptions/355 aliases/442 scopes сохраняют
preprocessed tokens, C17 syntax, ABI и register storage. Docs mapping PASS.
Отдельный stability200 проверил main/HEAD,32 dependency pins, donor, старый
packaged034 EXE и current generation inputs. Архив main200 сохраняет
код/docs/reports/generation без ROM/seeds/saves. Windows stage195/test037
не менялись, игра/GPU не запускались.

Границы: отдельный199 Source-only sceneFD test показал pending room request
BA0C4→BA05C/802170EC. Первый negative относился к ошибочному expected
Native timestamp при приостановленном мире; поправлен Source oracle и
добавлены once-only flag/request/descriptor/eligibility assertions. Второй
сценарий с уже установленным5B и без room request обнаружил model-time
расхождение после Source102→Native1: ожидается0, фактически30 на tick33.
Этот новый сценарий ещё не принят в основной fixture; negative logs/
traces сохранены, причина и исправление продолжаются отдельно. Никакая
проверка не ослаблена и runtime по этому расхождению пока не изменён.
Полный variable FPS и другие Source-only NPC/предметы/системы не завершены.

### Продолжение199c/201 — сюжетная передача NPC и новые понятные имена

Trace3 подтвердил причину Source26 state102→Native1: Original submission
на ceil(Source boundary) оставляет phase20 rational units после точного
766666666+40/60. Clip0 начинался766666667, а Source-loop wrap на1100ms
показывал model float30/flags0 вместо0; завершение сдвигалось в следующий
tick. В199c Source entry/post hooks26 сохраняют точную identity/lifetime/
overlay/table/tick/phase и один Native object scope включает только
submission ceiling<60 units (<1ns), возвращая команде исходную rational
boundary. Native-born reentry этим исправлением не заявлен.

199b quick PASS:8 settings,336 real constructors,1104 Source/240 Native
edge/208 reject schedules,7568 frozen189 complete comparisons,1440
intermediate yaw,3192 common NPC B. Строгие model time/command timestamps
сохранены.199c имеет те же runtime/rules/fixture плюс точные full counters;
шесть компонентных отчётов совпали с generation199c и исходниками. Отдельно
guards8 PASS192 actual Source closures: exact origin/once-only; stale
overlay/table/valid hidden replacement physics/model/state/kind reject
после пропущенного1ns; reset отменяет owner/command с сохранениемcontext,
heap/resources/models. Старые negative fixtures сохранены: active null
physics недопустим уже в Original до hook; другие player clip commands
были ошибочно отнесены к NPC; synthetic reset+reacquire менял реальные
heap identities при прежних размерах/models/refs, поэтому reentry-only
reset проверяет отмену, а не новый acquisition. Общий full199c ещё идёт.

Отдельный каталог194o PASS695 descriptions/360 aliases/448 scopes:
pickup idle yaw4, state10 detach/six actual effect constructors/fade/retire,
model-group opacity wrapper и opacity render flag имеют точный canonical
review/ROM/function/register hashes;96 preprocessed C units/C17/ABI/storage
совпадают. Failed194n (.fl вместо полного FPR storage) сохранён,194o
сохраняет весь исходный f2 storage. Эти5 новых имён пока не переносились
в main200; Native предметы/effect lifetimes не заявлены.

Изолированный201 продолжает Source-reviewed dialogue24 state3 и25 states2/
102: actual Original look proposal, signed return и original six-count,
20 новых close/immediate/wrap profiles на setting и actualD1498 command
timestamp. Его validation ещё идёт; main200 и замороженный199c не меняются.
Проверенная точка восстановления main200 сохраняется. Test037/Windows
stage195 остаются прежними. Игра/GPU не запускались, Windows build не создан.
Полный variable FPS, remaining NPC/event/pickup/platform/script-camera/
cutscene/minigame/2D blocks продолжаются.

### Source102→Native1 — точное возвращение NPC26 подключено (203)

Полный199c прошёл вместе со всеми прежними player/world/NPC B сценариями:
1008 actual constructors/288 skipped,3312 Source schedules,720 Native
edges/624 rejections,22704 frozen189 RAM/context/heap/resources/commands/
model-count comparisons и3192 common NPC B schedules. SceneFD проверен
с quest5B clear/set: настоящая BA0C4→BA05C сохраняет descriptor175/1,
once-only flag и pending room request; world eligibility остаётся Original.
При уже установленном5B нет room request, и Source dialogue102 возвращает
actor в Native idle1 с точным rational command origin, включая <1ns
Source submission ceiling. Строгая проверка model-time/loop flags сохранена.

192 дополнительных guard schedules отклоняют устаревший handoff при
смене overlay/table/physics/model/state/kind/lifetime, а reset отменяет его.
Архив199c хранит проверенные input hashes, generation, отчёты и guard proof.
Перенос в main203 подтверждён идентичностью всех96 canonical C units и
raw ABI files. Полный player report имеет явное origin199c; шесть main
компонентов проверяются отдельно после переноса. Каталог697 сохраняет
новые имена Source pickup helpers и actual mode4 contacts kinds24/25.
Все prior negatives/oracle corrections/source timing traces сохранены.

Native ownership26 dialogue102 пока не расширен: сюжетные item118/123
spawn branches остаются Original. Candidate201 ordinary24/25 dialogue
находится на полном CPU-прогоне; candidate202 проверяет actual Source
item/effect lifetimes отдельно. Windows stage195/test037 не менялись.
Игра/renderer/GPU не запускались; полный variable FPS продолжается.

### Main203 — Source/Natively owned NPC handoff проверен и сохранён

Все семь отчётов согласованы с текущими исходниками/generation203. Полный
player report явно сохраняет origin199c; шесть компонентов main203 выполнены
заново и прошли. Каталог697 descriptions/362 aliases/450 register scopes
сохраняет preprocessed C tokens, C17 syntax, ABI и ctx storage; docs mapping
совпадает. Stability203 подтвердил main/HEAD, все32 dependency pins и
чистое дерево donor. Код/docs/reports/generation203 архивированы без
ROM/seeds/saves. Windows stage195/test037 сохранены, нового EXE нет.
Candidate201 и Source item/effect research202 остаются отдельными; общий
перевод variable FPS продолжается и не объявлен завершённым.

### Source story pickups и имена — постоянная проверка в main205

Разобраны реальные descriptors/public kind table:118 Keycrystal,
123 StarPiece,127 Disk и130 GetFlash. Source branches NPC26(sceneAC/C1)
и NPC25(sceneC0) создают реальные actors в common pool; actual generic
constructor, resource lists/model binding/common updater и destructors
исполняются на synthetic assets. Frozen189 replay сравнивает полный
RAM/context/heap/resources/commands/model-count результата producer.
Семь success/already-set quest profiles ×24 settings прошли168 schedules:
96 полных item lifetimes и576 actual flash130 lifetimes. После96 Source
ticks весь созданный pool пуст, исходная heap/resource/model baseline
возвращается. Проверены обе ветви byte801B374A без угадывания его роли.

Проверка сохранена в tools/check_native_story_pickups.py и отдельном
research/native-story-pickup-source-check.cpp; main205 run прошёл с
ASan/UBSan/LSan.38 новых Source functions должны точно совпадать с
frozen189, любое неразобранное dependency останавливает checker.
Первое compile negative относилось к fixture-local descriptor/indentation;
следующее lifecycle assertion — к неверной six-model baseline из full
world fixture. Новый standalone fixture имеет ровно два player models;
heap/resources/lifetime требования не ослаблены. Negative artifacts сохранены.
Permanent-check preparation вызов исправлен, runtime не изменялся.

Каталог707 descriptions/372 aliases/455 register scopes: добавлены flash
delay/spiral/fade/retire, gravity/probe helpers и detach, понятные локальные
actor/physics имена только в доказанных инструкциях. Token/C17/ABI/storage
проверка выполнена непосредственно в main205. Docs/address mapping
обновлены. Все runtime/fixture/checker inputs прежних шести behavior reports
и96 canonical C units/raw ABI files идентичны main203; отчёты явно сохраняют
annotation-only relocation203 и полный player origin199c. Не повторялся
неизменённый часовой прогон только ради комментариев.

Границы: настройка FPS входит в24 settings; этот новый Source-only test не
заявляет Native trajectories/items130. Full-pool allocation failure, real
level assets/dialogue progression, Native NPC26 event-producing branches,
остальные NPC/camera/cutscenes/minigames/2D и весь variable FPS продолжаются.
Candidate201 ordinary24/25 dialogue всё ещё проходит полный общий run.
Main205 code/docs/reports/generation архивированы без ROM/seeds/saves.
Windows stage195/test037 не менялись, игра/renderer/GPU не запускались.

### Ordinary dialogue24/25 — полный201 перенесён в main207

Полный общий201 прошёл: kind24 state3 иkind25 states2/102 используют
настоящие Original proposals/geometry/clip commands, signed yaw return,
actual mode4 contact descriptors иonce-only completion marker. Проверены
3792 slot09 Source schedules/720 Native edge schedules/528 rejections,
26064 frozen189 full-RAM/context/heap/resources/commands/model comparisons,
1008 actual constructors/288 skipped и прежние3192 common NPC B schedules.
Runtime/fixture/checker inputs и96 canonical C units/raw ABI files после
переноса совпадают; main player report явно сохраняет origin201.

Каталог711 descriptions/376 aliases/462 scopes сохраняет новые имена
Source special24/28 иstory marker/counter bank3678. Bank3678 иbank3700
имеют разное исходное поведение sentinel-1; оно описано без угадывания
назначения конкретных игровых счётчиков. Постоянная Source pickup168
проверкаmain205 сохранена; её и шесть компонентов заново проверяем на207.

Границы: Native26 ordinary102 ещё проходит отдельный full206 с точными
AC/C1/FD quest guards. Special24/forced24/28 пока только в isolated208;
special28 state200, NPC26 event/item-producing closures, pickups/effects,
platform/script-camera/cutscene/minigame/2D и общий variable FPS продолжаются.
Windows stage195/test037 не менялись; новый EXE не создан, игра/GPU не запускались.

### Main207 — обычные dialogue24/25 проверены после переноса и сохранены

Все восемь текущих отчётов совпадают с исходниками иgeneration207. Полный
player/world report имеет проверенное byte-identical origin201; шесть
компонентов заново выполнены в main207 и прошли, постоянный Source quest
pickup/effect168 test тоже заново прошёл. Полный201 дал18176 промежуточных
slot09 yaw samples с signed return/wrap. Каталог711/376/462 сохраняет
preprocessed tokens/C17/ABI/ctx storage; docs mapping совпадает. Stability207
проверил main/HEAD, donor,32 dependency pins/clean tracked trees и старыйEXE.
Код/docs/reports/generation207 архивированы без ROM/seeds/saves.

Isolated206 обычный26 dialogue102 с guard поsceneAC10/C17B2F/FD5B ещё идёт
в полном прогоне. Isolated208 исследует forced24/28 иspecial24 с Source-end
quest/counter/sound: первый negative обнаружил поздний clip6 init у
Native-born73ms actor (Source3/Native0.57 к первому endpoint). Trace сохранил
точные команды106333333+20/60 и обе Original replays. В208b clip/dialogue
init снова interval-start, а initialized closure Source-end; настоящий
B91D4 сначала публикует busy1 (ROM91E4), поэтому event tail не выполняется
в init pass. Строгие model-time/commands проверки не ослаблялись,208b ещё
не подключён в main. Special28 state200/Native26 item/scene closure и
остальные variable FPS системы продолжаются. Windows stage195/test037
сохранены; нового EXE нет, игра/renderer/GPU не запускались.

### Ordinary dialogue26 — полный206 перенесён в main209

Полный общий206 прошёл: Native kind26 dialogue102 разрешён только при
точных Source scene/quest predicates:FD с5B,AC с10,C1 при!7B или2F.
Ветки175/1 иitem118/123 остаются Original; ownership проверяется каждый
pass. Actual mode4 EDF4 имеет пять новых descriptors/quest combinations,
включая5C; Source comparisons сохраняют Original flags/requests/commands.
Проверены4416 Source/720 Native edge/624 rejection schedules,30432 полных
frozen189 RAM/context/heap/resources/commands/model comparisons,24992
intermediate yaw samples,1008 actual constructors и3192 common NPC B.

Все runtime/fixture/checker inputs и96 canonical C units/raw ABI files
после переноса идентичны; main player report имеет явное origin206. Все
пять обычных NPC dialogue families24..28 этого overlay подключены.
Каталог711/376/462 сохранён;11 комментариев теперь описывают функцию без
устаревших Source-only/partial-ownership пометок. Original F920 очищает
selector44 вFB9C delay slot перед D1498; это закреплено в описании.

Шесть компонентов и постоянный Source story pickup168 test заново
проверяются на209. Special24/forced24/28 находятся в isolated210a:
strict quick passed1616 Source/11008 frozen comparisons/3192 common B;
288 pending-dialogue reset/Source fallback/hidden/reveal/state/lifetime/
latch rewrite guards прошли с реальными callbacks/resources. Полный210a
идёт; эти состояния ещё не включены в main209.208/208b model/init/publication
negatives и208c Source-born event→Native idle loop negative сохранены.
Source-only special28 state200/Native26 item/scene closure, предметы/эффекты,
platform/script-camera/cutscene/minigame/2D и весь variable FPS продолжаются.
Windows stage195/test037 сохранены, новый EXE не создан, игра/GPU не запускались.

### Main209 — пять обычных семейств Slot09 проверены и сохранены

Все восемь отчётов совпадают с текущими исходниками и generation209.
Player/world report имеет byte-identical validation origin206: 1008 actual
constructors, 4416 Source comparisons, 720 nonaligned command boundaries,
624 rejection cases, 30432 frozen side-effect assertions, 24992 intermediate
yaw samples, 3192 actual Native B/input cases; весь прежний player/world
suite также прошёл. Шесть компонентов заново прошли в main209. Постоянный
Source-only quest pickup/effect test заново прошёл 168 сценариев.

Обычный26 state102 принадлежит Native только в ветвях без публикации
события: sceneAC с quest10, sceneC1 без7B или с2F, sceneFD с5B; ветви,
создающие предмет или сценовую команду, остаются Source. Проверка повторяется
при каждом решении владения. Source102→Native1/101 reentry199c сохранён.
Каталог711 descriptions /376 semantic names /462 register scopes обновил
комментарии о текущем владении; preprocessed C tokens, C17, ABI и ctx storage
не изменились. Mapping docs актуальны. Stability209 проверил main/HEAD,
donor и32 pins/clean tracked dependencies, generation inputs и старыйEXE.

Isolated210a special24/forced24/28 ещё проходит полный прогон. Отдельные
288 cancellation/reset/hide/recreate/Source-fallback guards уже прошли;
negative208 traces сохранены, строгие model/command checks не ослаблялись.
Special28 state200, Native26 event/item closures и остальные variable FPS
системы остаются в работе. Windows stage195/test037 сохранены, новогоEXE
нет; игра, окно, renderer иGPU не запускались.

### Продолжение212 — special28 geometry/Source-end split, proof pending

Проверенный main209 сохранён в v209-slot09-ordinary-all-five-main-checkpoint.tar.gz,
SHA2568eec81e619b27b2682e295ba0b8acea9d5baa7c510e811c0626edbc1cc7b6f48.
Full210a special24/forced24/28 продолжает общий прогон; immutable snapshot8
и отдельный cancellation288 guard сохранены. Код main209 не изменялся.

Isolated212 добавил kind28 special200: геометрия вычисляется настоящими
E88C4/E8590/31A0C один раз на Source-период, yaw дробно продвигается в
actor10/physical4; полное Original FBAC решение выполняется только на
Source-end. FC3C JAL delay-slot write выполняется до подстановки target.
Source-body init сохраняет исходный yaw58, реальные sprite/count/quest4E/
completion/counter15 команды и их порядок; прогноз не публикует init или
сюжет раньше времени. Cache проверяет pointers/model lifetime/ROM/callback,
discrete fields/expected yaw/tick/denominator. Cancellation восстанавливает
только собственные неизменённые pose words; reset/Source fallback/ownership
loss очищают запись. Пять новых профилей включают Native-born73/173ms,
положительный/отрицательный wrap возврат и sentinel counter-1.

Первый Source-only negative212a показал неверное ожидание test oracle для
B50F4: bank3700 sentinel-1 нормализуется в0 до+1, поэтому результат1, а не255.
Это уже описано в каталоге; runtime не менялся, fixture исправлен в212b,
предыдущий fixture/log сохранены. ROM rule hash также должен соответствовать
ROM bytes, не C text; failed generation212 сохранена. Strict checks не
ослаблялись. Loop-only scene34 не имеет NPC; новые angle dependencies
объявлены fatal unvisited boundaries, реальные функции выполняются в
player/world fixture. Пять остальных компонентов212b прошли, loop повторяется;
quick endpoint proof и planned cancellation guard ещё не завершены.

Границы остаются явными: special28 не перенесён в main, Native26 event/item
closure, предметы/эффекты, остальные NPC/объекты/platforms, script cameras,
cutscenes, minigames и2D продолжаются. Windows stage195/test037 без изменений;
сборки на каждую правку не создаются, game/window/renderer/GPU не запускались.

### Special24/forced24/28 — полный210a перенесён в main211

Полный210a прошёл вместе с прежним player/world suite. Source-end event
decisions специальных24 иforced24/28 сохраняют реальные callbacks, Original
порядок иcommand timestamps. Special24 actual clip6 начинается непрерывно
в Native interval-start; настоящая B91D4 publication выполняется Source-end,
затем Original initialized body проверяет busy иonce-only quest51/counter14/
soundD0/completion. Pending clip/init record проверяет lifetime/tick/denominator/
ROM/private callback; reset/Source fallback/ownership loss очищают запись
и отменяют только собственный незавершённый latch. Negative208 traces сохранены.

Проверены4848 Source schedules,720 Native edge schedules,624 rejections,
33024 frozen189 full RAM/context/heap/resources/commands/model comparisons,
1008 actual constructors и3192 common NPC B. Отдельно288 cancellation guard
сценариев проверили reset, Source fallback, hide/reveal, state replacement,
actual retire/recreate lifetime иexplicit latch rewrite; completion/dialogue
once-only, ресурсы/model counts вернулись в baseline. Модель Native-born73/
173ms не теряет первый fractional clip interval. Full source snapshot8 неизменен.

Все runtime/fixture/checker inputs и96 canonical C units после переноса
идентичны; player report сохраняет явный origin210a. Свежие шесть компонентов
иSource-only pickup168 выполняются наmain211. Каталог711/376/462 и актуальные
role comments main209 сохранены. Candidate archive включает guard proof,
не включает ROM/seeds/saves. Новый Windows EXE не создан.

Isolated212b special28: quick1696 Source/11488 frozen comparisons/3936 yaw
samples иcancellation288 прошли; общий full идёт иещё не включён в main.
Native26 event/item branches иостальные variable FPS системы продолжаются.
Windows stage195/test037 сохранены, игра/окно/renderer/GPU не запускались.

### Special28 guard212b passed; sceneFD negative214 measured

212b full remains immutable and running. Quick passed1696 Source schedules,
11488 frozen189 full comparisons,3936 intermediate yaw samples and3192 B.
All six components matched212b and passed; loop-only fixture added fatal
unvisited E8590/31A0C dependencies, actual Original helpers are executed in
world fixture. Focused special28 guard passed288 Native schedules/48 Source/
288 frozen full comparisons/600 yaw samples at24 settings: reset, Source
fallback, hide/reveal, state replacement, actual recreation with new model
lifetime and external actor yaw write. Dialogue/completion occur once, final
resource/heap/model counts baseline. Receipt includes exact source hashes.

Full210a passed entire previous player/world suite:4848 Source schedules,
720 Native edges,624 rejects,33024 frozen comparisons,27456 yaw samples,
1008 constructors and3192 B, plus Source special24 cancellation288. Main211
integration preserved96 canonical C units/ABI/raw hooks and all runtime inputs;
fresh main211 verification is running. No Windows EXE/game/GPU was produced.

Isolated214 intentionally removed only sceneFD quest5B ownership rejection
to expose the event timing before any proposed fix. Strict Native60/Original30
kind26 scenario20 rejected at Source tick15: actor yaw328.872 vs325.106.
Detailed Original/Native command sequences have seven identical commands;
BA0C4(175,1) expected433333333+20/60ns, actual400000000+0/60ns: exactly one
Source30Hz period early. Clip5/clamp and following idle commands also shifted
one Source period. Both logs/fixture/receipt are preserved; no assertions or
canonical Source bodies were weakened,214 is not promotable.

ROM F254 callsB922C atF350, then sceneFD/quest5B selects the real flag/request
tailF360..F398. Actual request invalidates native gameplay eligibility. Current
live gates then skip common motion/model tailE8D88 and current() stops after
world before player/camera. A correct Source-end gate therefore also needs
completed-frame/handoff proof to prevent a second Source actor pass and lost
final motion/model age; these are investigation hypotheses until a controlled
fix and strict combined scene/actor/resource traces pass. Main keeps Source
ownership of this event branch. Native26 item118/123 closure and rest of full
variable FPS remain outstanding; existing test037 remains user's confirmed
dialogue-priority build, frequent microbuilds are not being made.

### Main211 — специальные24/forced24/28 проверены после переноса и сохранены

Все восемь текущих отчётов совпадают с исходниками иgeneration211. Full
player/world report имеет проверенное byte-identical origin210a; шесть
компонентов заново прошли наmain211, постоянный Source-only quest pickup/
effect168 test тоже заново прошёл. Каталог711/376/462 сохраняет Original
preprocessed C tokens, C17, ABI иctx register storage. Docs mapping актуальны.
Stability211 проверил main/HEAD, donor,32 dependency pins/clean tracked trees,
current generation inputs истарыйEXE. Код/docs/reports/generation211
архивированы безROM/seeds/saves; special24 cancellation288 proof сохранён.

Связанный NPC block продолжается. Isolated212b special28 full ещё в работе;
its Source/native quick and cancellation288 passed, но это не main code.
SceneFD Native event investigation214 deliberately exposes ungated event
timing: strict Source comparison remains required before promotion. Native26
item/event closure иостальные системы настоящего variable FPS не завершены.
Windows stage195/test037 сохранены; новогоEXE нет, game/window/renderer/GPU
не запускались. Никакие ветки/коммиты/pins илиdonor не менялись.

### Special28 — полный212b перенесён в main213

Последний особый Slot09 dialogue28 state200 сохраняет actual Original FBAC
в Source-end: вся инициализация, quest4E, bank3700 counter15, marker D1498,
completion и sprite lifecycle исполняются в исходном порядке. Непрерывный
look/return yaw использует реальные pure Original geometric proposals,
а callback получает восстановленный Source-start yaw. Проверены signed
return/wrap, Native births73/173ms и sentinel-1 counter normalization.
ROM/private callback/tick/denominator/actor/model/lifetime и raw yaw/fields
проверяются перед использованием; reset/Source fallback/ownership loss
отменяют только собственный незавершённый yaw.

Полный212b: 1008 actual constructors,5088 Source schedules,720 Native edge
schedules,624 rejections,34464 frozen189 comparisons,30488 intermediate yaw
samples,3192 actual common NPC B schedules и весь прежний player/world suite
прошли. Дополнительные288 cancellation/reset/hide/state/actual recreation/
explicit yaw rewrite schedules также прошли; Native contact publication и
completion происходят once-only, model/resource counts возвращаются в baseline.
Source snapshot8 неизменен. Parent210a full проверен до переноса.

96 canonical C units, frontier/hooks/overlay ABI и runtime/fixture/checker
inputs совпадают с212b; явный full validation origin сохранён. Каталог711
описаний/376 имён/462 scopes и актуальные комментарии main211 сохраняются.
Свежие шесть компонентов и Source-only story pickup168 проверяются на213.
Архив candidate включает288 guard proof безROM/seeds/saves.

SceneFD event/item Native closure остаётся isolated216: прототип ещё требует
доказать полный порядок оставшегося world/player/projectile/camera обновления,
возврат к Original и cancellation guards. Основной variable FPS продолжается:
остальные objects/effects/platforms/script cameras/cutscenes/minigames/2D.
Windows stage195/test037 не менялся, нового EXE нет; игра/GPU не запускались.

### Main213 — полный Slot09 special28 проверен после переноса и сохранён

Все восемь отчётов совпадают с текущими inputs и generation213. Полный
player/world report содержит byte-identical origin212b: 5088 Source/720
Native edge/624 rejection schedules,34464 frozen189 full RAM/context/
heap/resources/commands/model comparisons,30488 yaw samples,1008 constructors
и3192 common NPC B, вместе с прежним player/weapons/world suite. Шесть
компонентов заново прошли наmain213; Source-only pickup/effect168 тоже
заново прошёл. Каталог711 descriptions/376 names/462 scopes сохраняет
exact preprocessed C tokens/ABI/context storage; docs mapping актуальны.
Stability213 подтвердил main/HEAD/donor/32 pins/clean dependency tracked
trees/current generation inputs, packaged034 и037 неизменны. Код/docs/
reports/generation213 архивированы безROM/seeds/saves;288 special28 guards
сохранены в candidate archive.

NPC event/item Native closure остаётся в isolated216, полного proof ещё
нет. Остальные systems variable FPS продолжаются. Нового Windows EXE нет;
игра, окно, renderer/GPU не запускались; main/HEAD/pins/donor не менялись.

### Main217 — разобранные room-request функции названы, документация согласована

Каталог712 descriptions/377 names/467 register scopes. Новое имя
dora3_scene1_request_room_by_id_801BA0C4 объясняет ID→builtin descriptor→
BA05C request. BA05C описывает pending1/mode/descriptor globals, отсутствие
actor/model replacement и продолжение BB080. Пять конкретных register
access scopes названы; implicit forwarded a1 не имеет отдельного access
в wrapper и объяснён комментарием. Negative194t scope rejection сохранён.
Свежий names check подтвердил96 C units, identical preprocessed tokens,
C17, ABI/context storage. Все семь поведенческих отчётов сохраняют явный
annotation-only origin213 и совпадают с217/current inputs; full player/world
origin212b сохранён. Stability217: main/HEAD/donor/32 pins/EXE034/037 неизменны.
Docs mapping обновлён. Native scene event218/220 остаётся изолированным:
mixed218 выявил repeat соседнего NPC, completed-world-pass исправление220
проверяется. Main runtime не менялся, нового Windows EXE нет.

### Main217 — names-only checkpoint сохранён

Каталог712/377/467 и свежийnames proof сохранены с текущими восьмью matched
reports иgeneration217. Все behavior inputs/96 canonical C units/frontier/
hooks/overlay ABI идентичны213; семь поведенческих отчётов имеют явное
annotation-only relocation213, полный player/world origin212b сохранён.
Stability217 проверил donor/32pins/main/HEAD/unchanged EXE034/037.
SceneFD mixed218 negative иisolated220 исправление не включены в main.
Документация и имена переменных обновлены, Windows EXE не создавался.

### Git checkpoint217 — явный запрос пользователя2026-10-03

Пользователь уточнил, что локальные архивы не заменяют сохранение в существующий
Git repository. Подготовлен commit проверенного main217: variable FPS progress,
confirmed NPC B fix037, Slot09 ordinary/special/forced dialogues, читаемые имена,
Source-only pickup proof иактуальные/исторические reports сявными generation origins.
Краткий отчёт: docs/progress-2026-10-02.md. Isolated218/220 scene-transition
prototypes иnegative diagnostics остаются в ignored build-local, не входят вmain.
ROM/generated game C/assets/saves/keys исключены. Commit/push результат проверяется
по remote main после операции; публикация релизов/тегов и смена приватности не запрошены.


## Блок3 — сценарные камеры и катсцены (353, реализация готова к целевой проверке)

Выбран пользователем вместо следующего блока платформ. Native policy подключает
scene1 script-active для Original20/30Hz. Восемь Original script threads, opcode
dispatch, wait/pause/skip/cursors, создание/освобождение объектов, clips, звук,
диалоги и переходы исполняются на прежних Source updates. BB080 entry/end hooks
возвращают Source transforms перед командами и захватывают новый Source anchor
после всей цепочки, включая common actor/model update и eye geometry.

Каждый дополнительный кадр исполняет исходные equations участников, четырёх камер,
world origin и геометрии глаз в отдельном4MiB guest image с явным временем. Native
публикует XYZ/rotation/scale, camera target/eye/orbit/local/follow/distance и visual
alpha model group0. Это расчёт движения, а не interpolation двух готовых poses.
Source timers/flags/blink/physics/model lifecycle/registers/stack/RNG остаются
неизменны. Прыжки используют fractional gravity/drift и исходные collision queries;
плоскости мира и ceiling/floor не заменяются приближённым host ground plane.

Commands обрамляются настоящими Source hooks, включая equal-valued setters.
Наблюдаемые внешние controller changes сбрасывают anchor; actor/physics/model
identity guards запрещают восстановление прежних transforms в заменённый actor.
Pause/transition/resource barriers используют существующее завершение frame tasks.
Специальные overlay режимы/миниигры относятся к оставшемуся блоку4.

Каталог дополнен33 разобранными функциями и18 scoped register names; документы
функций/переменных/карта адресов обновляются генератором. Только после всего блока
проверяются его52 motion functions, interpreter/camera commands и новые boundaries,
плюс целостность имён и Windows compilation. Старые player/NPC/pickup matrices
не перезапускаются. Пока это implementation, а не PASS и не готовый EXE.

По окончании блока передать новый Windows build, commit/push в main и предложить
пользователю оставшиеся1:объекты/платформы,2:враги/AI,4:миниигры/спецрежимы,
5:2D/UI,6:общая интеграция. Следующий блок начинать только после выбора.
Full variable FPS игры ещё не завершён. Игра/GPU локально не запускаются.


## Блок5: общий2D/UI, generation355 — реализация перед проверкой

UI frame scope подключён к существующему game loop для scene1 gameplay/cutscene,
image и четырёх пар menu callbacks overlay2 (title/files/intro/selection). При
неподключённой мировой подсистеме отдельная UI policy строит Native кадры;
мировые Source callbacks продолжают исходные20/30Hz, их готовность не заявляется.
Восьми panel records добавлено дробное вычисление исходного quantized-rate
уравнения; screen rectangle — четыре float bounds с исходным source counter.
Четыре режима title letters выполняют actual guest math в isolated4MiB image:
kick/drift/cap, slide, exit/scale и alternate bounce; публикуются только visible
floats. Intro alpha, fixed selection actor sin bob/rotation, text row movement
пересчитываются по времени. UI не использует RT64 pose interpolation.

Фактический widgets draw продвигает reveal/lifetime при phase1; Native сохраняет
один шаг на Source epoch, дополнительные draws восстанавливают только widget
records. Dialogue arrow, files visual counters и selection panel growth тоже
сохраняют исходную частоту. Исходное число text-draw sound requests внутри первого Source draw сохранено; дополнительные кадры их не повторяют. Button/repeat,
dialogue/page/answer/busy, saves/pak/EEPROM, transitions, waits/finish flags
остаются Original. Временные visible fields восстановлены до следующего Source
update; callback pairs требуют правильного loaded overlay, divider2/3 и
безопасной framebuffer dependency. Старые player/NPC/dialogue fixes сохранены.

Точные ROM sites/body identities, понятные имена/роли/комментарии и документы
обновлены. Следующая проверка — только связанный UI блок перед Windows040;
неизменённые player/NPC/weapon/pickup/world suites не запускать. Игру/GPU не
запускать. Реальный вид меню, текстур/шрифтов и прохождение проверяет пользователь.
Полный variable FPS всей игры ещё требует блоков1/2/4/6. По завершении5 предложить
эти блоки на выбор, следующий самостоятельно не начинать.

Source dispatcher отмечает фактически вызванные UI domains. Пауза или ветвь,
пропустившая panels/rectangle/title/intro/text math, не получает дробного шага.
При повторном draw берётся состояние перед соответствующим Original вызовом,
а после него восстанавливается уже сохранённый Source результат. Это сохраняет
не только счётчики, но и тот же видимый text reveal; несколько views сохраняют
исходное число шагов. UI clock использует собственное Source wall-time.

Полный разбор63B8 уточнил роль: это arrow texture builder с blink и sound,
а не только звук. Дополнительные Native кадры повторяют geometry/commands;
только JAL444C0 пропускается, blink/sound flags получают Original pre-draw
состояние и затем Source результат восстанавливается. Каталог/документы исправлены.


## Блок6 — интеграция, готовый код перед общим CPU прогоном

Native часы world/scripted/minigames/UI/models запускаются после фактического
Acquiring→Native, когда завершилось чтение Original кадра. Сессия принадлежит
точной паре update/draw, ROM, Source divider и монотонной verified load generation,
включая перезагрузку родительского overlay. Старый wake не получает новый owner.
Один frame timestamp используется движением, камерой, моделями, UI и fade.

Manual→Original сохраняет физическую дробную фазу клипа. Отдельное продолжение
Source-view сохраняет исходные дискретные equality/clip-end решения врагов:
18→21 не изобретает frame20. Неполученные непрерывные cues доставляются один раз;
pause/run смена сохраняет owner, seek/rebind/free прекращают старое продолжение.
Renderer bookkeeping reset не удаляет живую анимацию. Foreign RAM writes и
replacement minigame modules не получают старые borrowed поля.

Резервные player states19/23/26/30 — восемь точных8-byte jr-ra/nop handlers.
Они не требуют fallback всего мира; state0 остаётся Source character/resource
initialization. Pak/file UI BB52C/BB648 ROM635C0 подключён к Native UI: authored
scalar easing таблицы курсора/размеров, integer pixel truncation, hint offset,
isolated sprite/bob draw cadence; device polling/FSM/I/O выполняются Source once.

Callback inventory включает все ordinary process-pair installs. Четыре
auxiliary пары остаются явными Source boundaries: ROM59E80 4FE4/5064
(startup/device state machine с командами в draw),59D8/5A70 (actor/camera
control),6FD0/70A8 (resource/model selector), ROM70A50 F9B58/F9C88
(отдельный actor/model control initializer с heap80385000). Это не игровые
миниигры и не normal world pair. Их Native движение не заявляется;
строки fallback называют точную причину. Неизвестные callbacks, loader,
framebuffer copy, дополнительные процессы и resource/pause границы также
сохраняют Original. Callback scan — candidates; реальные ROM тела и load
guards являются основанием решения. Это не доказательство полного прохождения.

Подготовлен общий connected CPU набор31 checks, включая actual session/loop
integration, delayed activation, двухбанковую передачу, stale wakes/reloads,
models/cues и все прежние блоки. Проверки/Windows046 запускаются только после
этого целого кода/имён/документов. Игра/renderer/GPU локально не запускаются.


Пачка1 после048: реализованы palette-equivalent water scroll, общий type5
numeric angle reset (menu background0A46 и clip0489), Source cadence выбора
в диалоге. Код/семантические имена/документы готовы, проверки и EXE НЕ запускались:
пользователь сначала передаёт все пачки, затем явно говорит «нет».
Подробности и границы: docs/native-regressions-after048-batches.md.


Пачка2 после048: исправлены пропуски common белого fade в scene34 destinations/
states, world holds/paused scripts/room transitions и последнем count1 интервале.
Interior actual cleanup/load получили ROM-pinned resource barriers; реальные
Source decisions сохраняются. Подробности: docs/native-regressions-after048-batches.md.
Тесты/генерация/EXE не запускались: ждём окончания всех пачек и явного «нет».


2026-10-06, пачка3 после048: исправлен пропуск четвёртой area карты и границы
descriptor table24. Общий bitmap adapter курсора/значков компенсирует RT64
fractional-top UV bias, ограничивает valid texels и нормализует last-strip extent.
Статический ROM/renderer разбор — research/native-map-sampling-audit.json;
подробности/неисполненные будущие cases — docs/native-regressions-after048-batches.md.
Новые тесты/генерация/EXE пока НЕ запускались: ждём завершения всех пачек от пользователя.
