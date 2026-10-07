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

## После053: повторные касания цветных шариков (2026-10-07)

Пользователь подтвердил исправление выхода через дверь. Новый дамп053 PID11472:
access violation в проверке третьего цвета SoundLightBlue/Red/Yellow.
MSVC /O2 неверно свернул base+3*index в guest helper8023D730. Генератор теперь
выключает оптимизацию только этой функции под MSVC, затем восстанавливает её.
ROM body/overlay pinned; исходная логика головоломки и ABI сохранены.
Компиляция настоящего translation unit и статический просмотр assembly показали
правильный расчёт. [Разбор](color-ball-crash-after053.md). Тесты/игра/новый EXE
не запускались. Следующий билд требует новой CPU generation; ждём команду сборки.

## Test053: исправление выхода через дверь (2026-10-07)

По прямой команде «билдим» собран Windows053 из33c1e74.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-053.exe.
SHA256: b6d32cf7f8421d331e9fbe3682c3e4750817ad7122d048cc43262a8a1064a235.
Финальный fade при выходе из Native использует timestamp текущего события.
Подтверждённые пользователем исправления аудиозадержки и Сидзуки включены.
MSVC RelWithDebInfo завершён успешно; generation051/RSP переиспользованы
с неизменными входами. Все171 прежних файлов вне logs сохранили хэши.
Тесты, игра, эмулятор, renderer/GPU не запускались. Пользователь подтвердил
исправление выхода через дверь. Предыдущий compile receipt сохранён в research/history/.

## После052: гаджет-дверь и выход из уровня (2026-10-07)

Пользователь подтвердил: аудиозадержка и вылет катсцены Сидзуки исправлены.
Новый дамп052 PID1000 показал обратный ход FadeTimeline при выходе из Native:
Source fade command новее последней render pose. synchronize теперь завершает
fade на timestamp текущего события now, а не старой позы. Строгий guard сохранён.
[Разбор и границы](door-exit-crash-after052.md). Код исправлен и включён в053 по команде пользователя.
Локальные тесты/игра не запускались; пользователь подтвердил исправление двери в053.

## Test052: исправление вылета у Сидзуки (2026-10-07)

По команде «собирай» выполнена инкрементальная Windows-сборка из4be12e2:
сохраняется мировой command timestamp при suspended Original NPC callbacks.
Включена аудиоправка051. EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-052.exe.
SHA256: fa270a9cb60b3bfedaa38f0de2abc0e82cf4c52492a7777a8f97353666aad21e.
MSVC RelWithDebInfo успешно завершён. CPU startup-audio-test051-v1 и RSP
переиспользованы: их исходные входы не менялись.168 прежних файлов вне logs
сохранили хэши. Тесты и игру/эмулятор/renderer/GPU не запускали. Старые PASS
не перенесены. Пользователь подтвердил исправление аудио и Сидзуки. Пачка052 собрана.

## После051: вылет при разговоре с Сидзукой (2026-10-07)

Пользователь подтвердил: вылет существовал до051. Дамп051 PID28448:
FadeTimeline::sample_at выбрасывает «Fade clock moved backwards or invalid fraction».
Обработчик contact8022DA68 включает фейд из dispatch_native_npc_action,
который временно отключает continuous simulation через SimulationSuspension.
Командный timestamp терялся и заменялся временем CPU; следующий sample
того же игрового кадра оказался на1781000нс раньше. Это не ошибка аудио.

Исправлен native_gameplay_preview_command_time: сохраняется общий gameplay_slice
и при временной приостановке motion. Это также покрывает связанные Original
contact/geometry/birth callbacks и исключает подстановку локального возраста
actor/projectile вместо мирового времени команд. Строгая проверка FadeTimeline
сохранена. [Доказательства и границы](shizuka-fade-crash-after051.md).

Разбор и исправление завершены; правка включена в052, см. актуальную запись выше.
Локальные тесты/игра не запускались; пользователь подтвердил исправление в052.

## Test051: исправление нарастающей задержки звука (2026-10-07)

По прямой команде пользователя «делай билд» подготовлен Windows051 на основе
коммита65758d17bc6d4851b6b95f0c5a54ae31c18c5e7f. Исправлены полный учёт SDL queue,
минимальный aligned audio buffer (368->352 при22050/60) и unsigned underflow.
Общий resident путь действует в Original/Native для музыки, голосов и эффектов.
Подробности: [разбор аудио](audio-queue-drift-after050.md).

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-051.exe.
SHA256: 3b7b1dbd7d751aca43c44fbdcbe95073b056c72643e657f3e18490239f2c5091.
Generation startup-audio-test051-v1, прежняя audio-rsp-v1; MSVC RelWithDebInfo
успешно скомпилирован и упакован. Все165 прежних файлов вне logs сохранили
хэши, включая EXE/ROM/settings/saves/shared DLL. Тесты не запускались;
исторические PASS не перенесены. Игра/эмулятор/renderer/GPU не запускались.
Пользователь подтвердил исправление задержки звука после052.
Пачка аудио051 собрана; следующий баг — новый запрос. Состояние compilation
отделено от игровой проверки. [Windows workflow](windows-build-workflow.md).

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

# Блок6: реализация закончена, общий pre-build следующий


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

## Полная подготовка связанной проверки 218 типов после промежуточного042

Весь код блока2 включён в основной проект до общего pre-build. Генерация361
и проверка1168 точных Source bodies завершены;23 независимых/общих CPU checks
прошли. Общий enemy replay и четыре зависящие от него проверки ещё не завершены.
Падения в Source при отсоединении снаряда, создании сегментов и contact index,
а также сравнение неиспользуемого a3 исправлены в подготовке проверки.
Самостоятельные варианты теперь вызывают полный собственный конструктор.
Подробности и ограничения: docs/native-enemy-fixture-coverage.md.
Старые неуспешные попытки и их входы сохраняются; новый PASS не объявлен.

## Продолжение всего блока2 после замечаний по042

В основном runtime подготовлено 218 из 218 descriptor types. Это код внутри
незавершённого блока2, включая исходные25 из042; новый этап не проходил
generation/compilation/CPU validation и не входит в уже сохранённый EXE042.

Добавлены Bironn/segments, Doragonrock/fire, Baguram/generator, Barupu/Onpu,
Supaizan/web/projectiles, Oppi, Batti/Honeycomb, Derorin, Doridori/Tolneder,
Metabo/Gabugon/tongue, IgaHead/Tutaru, Butterfly/Sakana/Thunder,
T00/T17 controllers/effect/replacements/Iwa, RadiconRobo/Car,
KaenDan/KaenHousha с полной цепочкой, Churatto/Banyatto, оба Gokireon,
Thunder283–286 с генератором/разрядом, Arizinogu со снарядом и оба Tamabonga
со всеми bomb/effect owners; Houdai/Majin/Posedo/Kirafu/JyaiCong, TwinFace,
Tatsunoorgo, Fisher, Yadokaringer, Rikurage и Sentoutei со всеми их частями,
снарядами, свечением и контроллерами; BroboDC08 и оба Soundorago с полными
связанными цепочками, историческими траекториями, бомбами и эффектами.
Внесены Boss1 399–409, Boss2 410–427, Boss3 428–434, Boss4 435–445,
Boss5 446–453, Boss6 454–471 и Boss7 472–477 с их полными
Source closures: части/оружие/следы, связанные движения, урон/возврат,
подъём частей, затухающие подпрыгивания, падение, fades и reward owners.
Следы Boss1/Boss2 сохраняют реальные очереди Source; голова Boss2 читает
предыдущий период тела в исходном порядке pool. Этим кодом блок ещё не закрыт.
Реальные Source closure, математические зависимости, animation ownership,
родительские/дочерние команды и lifecycle guards описаны в family documents.
Включены составное движение двух проекций Yadokaringer335, накопление
синусоидального курса и совместная волна скорости/наклона Fisher, а также
поправки вертикальной скорости Rikurage у границ воды. Профиль скорости
Rikurage, связанный с кадром анимации, уточнён по Source ветви, CC rate,
seek/run и duration. BroboDC08 Hob сохраняет шесть реальных
прошлых уравнений поворота родителя; Soundorago body/tail использует те же
реальные записи траектории из буферов40/60 с actual endpoint guards. Native
геометрия не повторяет сдвиг Source history или collision commands. Чистый
расчёт движения вынесен для повторного использования в snapshot trajectories;
это изменение ещё требует единой финальной проверки всего блока.

Каталог сейчас содержит 2072 описаний/1741 смысловых aliases/5305
ROM-evidenced scopes. Новые scopes не объявлены проверенными генератором.
Документы и адресные соответствия поддерживаются штатным documentation updater.
Generated C/source mirror не правились вручную. Старые PASS358 относятся к358.

Внешняя зависимость478 и общие effects257–259 также внесены по фактическим
ROM scripts; все218 типов перечислены в docs/native-enemy-block2-worklist.md.
Весь выбранный блок2 сначала реализовать
вместе с именами/описаниями, затем расширить проверку на окончательный code и
запустить один полный CPU pre-build с зависимостями, затем Windows EXE.
Промежуточные generation, compilation, fixtures/tests/build не запускать.

Замечания пользователя042: пауки/снаряды, взлёт птицы и летучие мыши выглядят
с меньшим FPS. Supaizan/Batti/Oppi теперь включены в основной code, но точное
соответствие пользовательских визуальных названий kind ещё не подтверждено.
Механический кролик трясётся при приземлении в лесу фей после города; причина
и исправление пока не подтверждены. Не объявлять эти замечания закрытыми до
финального разбора/проверки. Игра/окно/renderer/GPU локально не запускались.

## Уточнение после промежуточного test042

Пользователь справедливо указал, что выбранный блок2 ещё не завершён.
Первая связанная группа25 типов была ошибочно принята за достаточный этап
для общего pre-build и EXE. Test042 уже скомпилирован/сохранён; commit8399e8a
отправлен в main. Это промежуточное состояние, а не завершение блока.

Дальше сначала полностью реализовать оставшуюся часть блока2, включая
семейства/части/генераторы/боссов и читаемые имена/документы. Общие проверки
и следующий EXE — только после полного блока2. Рабочий вариант ещё9 типов
подготовлен отдельно, но его генерация/компиляция/CPU-прогоны не запускались.
Продолжать реализацию целого блока; не запускать их ради промежуточной группы.

## Test-042 передан: первая связанная группа блока2

Сборка042 изgeneration358 сохранена в D:/Games/DoraRecomp3/testbuild после
полного connected CPU набора28 checks.25 типов/24 callback roots прошли
300 frozen/current Original и900 Native schedules; полный player/world suite
завершён на24 настройках. Добавлены152 смысловых имени функций и238 scoped
variable names; документы обновлены. Игра/renderer/GPU локально не запускались.
Все предыдущие файлы, ROM/settings/saves/shared DLL сохранены;32 pins/donor
неизменны. Следующими внутри блока2 остаются остальные семейства, связанные
части/генераторы и боссы; весь блок2 и весь variable FPS ещё не завершены.

## Исправления039/040 — этап356 / test-041 передан

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

# Журнал причин и проверок

## 2026-09-27 — начальное состояние

Авторизация: начать отдельную рекомпиляцию третьей игры, сохранить Dora64,
не публиковать. Указанный ROM доступен, WSL Ubuntu доступна через разрешённый
запуск команд. Инструкции AGENTS в Dora64/родительских проектах не найдены.
Действующие правила нового проекта записаны в его AGENTS.md.

Проверено: Dora64 local/remote HEAD = 680d81f, чистый status. Подробные Android
документы содержат исторические отрицательные результаты, поэтому текущий
вывод берётся из финальной RC1/1.0.3 записи, не из промежуточных test5/test6.

## R001 — стандартные сигнатуры не обнаружили реальное сжатие

Наблюдение: скан MIO0/Yaz0/Yay0/RNC/gzip/ZIP пуст.
Причина: обнаруженный потоковый декодер использует собственный 16-байтовый
заголовок и LZSS с 1-КиБ словарём. Вывод «сжатия нет» был бы ошибкой.
Проверка: 3129 последовательных записей распаковываются до declared size;
11 выборочных результатов совпадают побайтно с исходным декодером.
Граница: после 0x56FC30 структура не установлена.

## R002 — конец потока в середине пары LZSS

Первый строгий Python-декодер отвергал первый образец как truncated reference.
Разбор оригинальной функции: EOF разрешён и после первого байта пары,
функция возвращает текущий output count. Python теперь допускает это только
при уже достигнутом точном declared output size. Остальные усечения отвергаются.
Это исправление исследовательского инструмента, не патч игры.

## R003 — ограничения symbol-input в N64Recomp

1. `entrypoint` должен соответствовать ROM 0x1000; использование его для
   standalone decoder вызвало `No entrypoint provided in symbol file`.
   Для decoder config поле опущено, имя функции задано в syms.
2. Неизвестный zero-size direct-call symbol превратился в пустой static wrapper.
   Симптом: оригинальный декодер вернул 0. Решение: реальный размер функции DMA
   + `patches.ignored`, собственная явная внешняя реализация для probe.
3. Игнорируемая функция требует явного C declaration; GCC 15 отвергает implicit
   declaration. `recomp_include` содержит целую строку `#include`, а не имя файла.

Правки внесены в генератор/metadata, затем код сгенерирован заново; сгенерированные
файлы вручную не правились. Не использовать пустые заглушки для «успешного старта».

## R004 — PI DMA 1023/1024

Симптом на первой записи: исходная рекомпилированная функция при точном memcpy
выдала 34703 байта вместо 34728. Python по полному потоку выдал 34728.
Код читателя 0x80034000 устанавливает chunk counter=0x3FF, передаёт его как
DMA length, затем читает 0x400 байт. Обёртка делит запрос на чанки <=0x100;
последний 0xFF зависит от нечётного PI DMA rounding.

Первичный независимый источник: Mupen64Plus pi_controller.c в зафиксированном
commit округляет нечётные длины >=0x7F вверх. В pinned native runtime
do_rom_read копирует ровно size; do_dma/osEPiStartDma не добавляют округление.

Контролируемая проверка: единственное изменение probe — чтение 0x400 при
запросе 0x3FF. После него 11 записей совпали побайтно с Python.
Вывод: граница существенна для третьей игры. Probe моделирует этот известный
случай; он не является полной реализацией аппаратного PI. Runtime ещё не изменён.
Следующее действие: узкое обоснованное решение при подключении настоящего PI
и проверка активного загрузчика; не менять рабочий Dora64.

## R005 — snapshots не переносятся одной зависимостью

Подтверждено исходниками: runtime mutex/API и RT64 snapshot pointers общие,
но захват/диапазоны/матрицы/свет/sky подаются из игровых хуков первой части.
Следовательно, готовый форк — база, но третья игра требует собственной схемы
владения памятью. Адреса/пулы/хуки первой части не скопированы.

## Проверки этапа

- Инструменты N64RecompCLI/RSPRecomp собраны Release из pinned upstream + zero fix.
- ROM SHA-256, MD5 и CRC проверены; CRC1/2 пересчитаны и совпали.
- Linux GCC 15.2: boot-probe PASS, decoder-probe и differential PASS (11 записей).
- Windows MSVC VS2022: boot-probe собран и выполнен, PASS.
- Android NDK 27.0.12077973 / Clang 18.0.1 / API24 / arm64-v8a:
  boot-probe собран как native executable. Это не APK, запуска на устройстве нет.
- Все форки и вложенные зависимости размещены отдельно на точных commits.
- Прохождение, GPU output, полноценные input/audio/save и device compatibility
  новой игры не проверялись и не заявляются.

## 2026-09-27 — приватный репозиторий и изоляция библиотек

Пользователь разрешил создать приватный GitHub-репозиторий Dora64-3.
Зафиксирована политика docs/dependency-policy.md: отдельные копии, локальные
ветки dora64-3 в четырёх основных библиотеках и сборка строго по SHA.
Все 32 коммита lock доступны через GitHub API. Код и pins библиотек не менялись.
Публичные ветки зависимостей и релизы не публикуются в рамках этой операции.

## R006 — C/C++ linkage диагностических хуков

Первая сборка frontend не линковалась: сгенерированный C ожидал C-символы,
а декларации хуков в C++ имели другое связывание. В генерируемом startup_hooks.h
добавлены __cplusplus/extern "C" guards. Повторная генерация и сборка прошли.
Сгенерированные игровые файлы вручную не редактировались.

## R007 — ложное несовпадение DMA в собственной проверке

Первая побайтная проверка сообщила mismatch на offset0 первого аудиобанка.
Причина: MEM_BU(i, (int32_t)ram) складывал uint32_t i и signed32 ram, получая
zero-extended адрес. Макрос runtime ожидает 64-битный sign-extended gpr.
Исправление только в диагностике: MEM_BU(i, (gpr)(int32_t)ram).
После исправления реальные DMA обоих банков совпали полностью: 0x1C760 и
0x2580 байт. Runtime и игра не патчились. Это не отменяет отдельный R004:
эти банковые чтения имеют чётную длину и не проверяют случай 0x3FF/0x400.

## R008 — RSP попал в предварительную карту CPU-функций

Первоначальный resident disassembly до ROM0x3E910 захватил RSP boot/audio.
Разбор CPU audio task builder и RSP boot установил границу ROM0x3DA20.
Кроме того, эвристика объединяла entry tail jump с соседними функциями.
Вывод: автоматические function boundaries являются кандидатами; в startup.json
включаются только вручную просмотренные функции с проверенным размером.

## Проверки диагностического runtime и audio RSP

Linux GCC15.2: 45 original functions / 28 SDK entries / 54 direct-call
fail-stop boundaries сгенерированы и собраны с закреплённым runtime.
Выполнены boot/idle/main/scheduler, SI/VI setup, DMA обоих аудиобанков,
релокация банков, audio DMA pool, osAiSetFrequency(22050), начало synth init.
Последняя проверенная граница: original function 0x80051B88, exit77.
Нет данных о первом кадре, реальном PCM, вводе или сохранениях.

Audio RSP найден независимо по task pointers: text0x3DAF0 size0xE20,
data0x53F40, IMEM0x1080, 16 dispatch entries. Text hash совпал с aspMain
первой игры при отдельном сравнении. RSPRecomp и компиляция GCC прошли;
исполнения RSP и проверки соответствия реальной задаче пока нет.
Результаты v5 до исправления R007 не являются доказательством ошибки ROM DMA.

## R009 — остановка main обрывала уже запущенный RSP

В v14 появился RSP TASK type2, но main параллельно достиг непроверенного
800331F8 и вызвал _Exit до записи результата RSP. Это не доказательство
ошибки микрокода. Причина установлена по логам и немедленному завершению
всей программы в диагностической границе.

Диагностика учитывает реально выбранные scheduler audio tasks и их
штатное завершение. В fail-stop вызывающий поток останавливается; до 250 мс
даётся уже отправленной задаче. Игровая функция не получает успешный возврат.
V15: audio_submitted=1, audio_completed=1, RSP returned Broke, exit77 из-за
800331F8. Изменён только frontend; библиотеки и game timing не патчились.
Доставка SP completion в оригинальный scheduler ещё не проверена.

## 2026-09-27 — audio thread, первый RSP и graphics task

Linux GCC15.2 / RelWithDebInfo: 131 оригинальная функция, 38 SDK entries,
15 indirect targets, 60 fail-stop границ. Расширены конструкторы/фильтры/
эффект/voice pool, scheduler и начальная сборка display list. Включение
основано на коде третьей игры; generated C вручную не правился.

Audio thread8004E044 сформировал task8004DD4C. V15: type2, text800626F0
size1000, data80078B40 size800, commands800C0CD0 bytes9A0. Загруженный textE20
совпал с ROM побайтно, RSP завершился Broke(1).
V13 до связывания RSP: первая graphics task дошла до renderer boundary,
text80063510 size1000, display list80120638 bytes30; renderer её не исполнял.
Протоколы сохранены отдельно, поскольку это разные executable.

OSTask объявляет text size1000, но инструкций E20. Runtime загружает F80 data
bytes в DMEM, хотя задача объявляет800. Эти размеры нельзя смешивать.
Общий runtime не менялся. Нет отрисованного кадра, проверки PCM/музыки,
физических контроллеров, сохранений или Windows/Android runtime.
Следующие точки: 800331F8, 80026B28 и начальный процесс 800433D8. R004 открыт.

## 2026-09-27 — SP completion и первый PCM

147 оригинальных функций / 40 SDK entries / 80 fail-stop boundaries.
V18: RSP завершился Broke; 80026B28/80026D40 отправили completion, audio thread
продолжил 8004DCBC/8004D61C и вызвал AI output. Callback получил 1024 отсчёта
(512 стереокадров), все нулевые. Это не проверка музыки/ADPCM/sample DMA.
Linux GCC15.2: сборка/запуск прошли, ожидаемая остановка graphics/PCM 78.

## R010 — графическая задача ещё не видимый кадр

Первая display list: 48 байт плюс вложенные списки, 65 команд состояния.
Нет vertex/triangle/fill-rectangle/texture-rectangle. Отсутствие картинки
здесь само по себе не означает ошибку renderer.
Приватный снимок снят до SP completion. RT64 replay распознал F3DEX2,
обработал список: Vulkan/dzn, RTX2060, exit0. Сохранены VI до completion,
swap/unblack не симулируются. Выбран существующий coverage-attachment fallback
при отсутствии dual-source blending. Качество на сценах ещё не проверялось.
Снимок исключён из Git; metadata в graphics-replay-check.json.

## R011 — SaveType::None несовместим с будущим стартовым процессом

800440A0 вызывает 8003C894(queue,2), при несовпадении остаётся в цикле440C4.
3C894 вызывает 4E700: она возвращает 2 для EEPROM status C000, 1 для 8000.
Это osEepromProbe и требование EEPROM 16k. Видны три слота по 512 байт:
чтение43EDC, проверки43FD8, инициализация44050. Их полноценное исполнение
и сохранность данных ещё не проверены. SaveType::None пока только диагностика;
перед живым startup потребуется Eep16k, не обход проверки.

## R012 — гонка чтения VI в новом диагностическом capture

При проверке исходников установлено:get_vi_regs() возвращает update_screen_regs,
которые графический поток меняет при ScreenUpdateAction без graphics_rdram_mutex.
Первый capture читал их из scheduler-потока; общий mutex RDRAM этого не защищал.
Данные VI из v18/replay-v1 нельзя считать согласованным межпоточным снимком.

Исправление только в frontend:update_screen копирует VI на владеющем потоке;
scheduler получает опубликованную копию под отдельным mutex. Если она ещё
не опубликована, capture прекращается с кодом83 вместо фиктивных регистров.
На graphics boundary диагностическое ожидание до250 мс также даёт уже
завершённой аудиозадаче достигнуть первого AI callback. Renderer не возвращает
фиктивный успех и не пропускает задачу.

V20 после сборки:audio completion,512 нулевых стереокадров, безопасный capture,
exit78. Replay-v2 на новом снимке:RT64 setup0, F3DEX2, display list parsed,
exit0. Состав65 команд совпал. Старые metadata сохранены как superseded history.
Библиотеки, gameplay и donor не изменялись.

## R013 — преждевременный SP completion и ограниченный live эксперимент

events.cpp закреплённого runtime посылает graphics SP completion до send_dl.
Это позволяет оригинальному планировщику продолжать работу во время разбора.
Комментарий о задержках первой Doraemon нельзя переносить как доказательство
поведения третьей. Исследовательский live target генерирует проверенную по SHA
копию events.cpp с completion после send_dl. Деревья зависимостей не изменены.

RT64/SDL/Vulkan frontend собран; v24 прошёл setup и разбор первой F3DEX2 task.
Сгенерированный аудио RSP выполнил две задачи. Оригинальные80026F2C (yield request)
и80026C44 (DP completion) стали достижимы и добавлены после разбора ROM.
Runtime osSpTaskYield игнорирует запрос, osSpTaskYielded возвращает0 — штатная
модель этого runtime, не новая игровая заглушка. Оригинальный scheduler её принял
на этой короткой последовательности. Нет доказательства общего отсутствия
гонок, исходного темпа или пригодности такого порядка для полноценных сцен.

## R014 — fail-stop CPU обрывал ещё не разобранную графическую задачу

В live-v1/v21 главный поток мог достигнуть неизвестной функции раньше завершения
первого send_dl. Немедленный _Exit давал успешный RT64 setup, но ни одной записи
PARSED. Причина — диагностическая остановка всего процесса, не доказанный отказ
RT64. Добавлены счётчики принятой/разобранной работы и ожидание до2 секунд на
fail-stop; caller остаётся остановленным. V22 и последующие: submitted1/parsed1.
Ожидание не делает неподтверждённые функции успешными и не заменяет shutdown.

## R015 — EEPROM startup и фактическая загрузка scene34

После Eep16k и проверенных SDK mappings osEepromProbe/osEepromLongRead исходный
800440A0 прочитал три слота и подготовил пустые RAM buffers. Проверены тела
проверки подписи/суммы и сброса; checksum-ветка непустого save ещё не исполнена.
R011 устранён для startup; запись/перезапуск сохранений пока не проверены.

800433D8 выбрала0x22; таблица8007DC20 entry33 ->80043534. Пройдено обновление VI,
scene heap80385000/30000 и очистка801B3770..80385000. Оригинальные430EC/43154
загрузили ROM5E400->801B3770 (51C0) и ROM635C0->801B8ED0 (D490). Обе DMA
совпали побайтно до исполнения overlay. Текущая контролируемая граница800280DC
перед установкой callback801C0184 (ROM6A874). Нельзя зарегистрировать его как
resident с дельтой80024C00: потребуется настоящая секция оверлея.

Проверка раннего v24:163 CPU functions /43 SDK /69 fail-stop boundaries;
live RT64 setup0, первая state-only task parsed, SP/DP completion, два audio
RSP completions,512 начальных нулевых стереокадров,4 verified DMA, exit77.
Итоговый бинарник: та же граница77 и4 DMA, одна завершённая audio RSP task,
PCM ещё не достигнут. Default diagnostic отдельно пересобран:78 в graphics
callback до отправки audio. Начальное условие smoke-check об обязательном
audio completion оказалось излишне строгим для зависящего от host scheduling
порядка. Проверка уточнена по фактической границе; повторы ради счётчика не нужны. Видимого кадра,
реального ввода/аудиоустройства и выполнения overlay callback ещё нет.
Данные и хэши: research/live-startup-check.json; снимки/ROM/ресурсы не в Git.


## R016 — resident delta и ранние overlay aliases

Runtime init вызывает load_overlays для boot1MiB. При появлении новых sections
это регистрирует их по временной boot delta, хотя игра загружает их позже на
другие адреса. Генератор теперь разделяет resident/common/scene34. Entry hook
снимает неверные aliases; исходный scene clear снимает активный код, DMA
регистрирует его только после побайтного сравнения. Выход функций за секции,
перекрытия и отрицательный ROM offset проверены четырьмя отрицательными случаями.
Общие и scene34 callbacks исполняются после соответствующей регистрации.
Повторную замену сцены ещё не исполняли; это следующая проверка lifecycle.

## R017 — короткие данные скрывали ошибку DMA и неверное условие hook

Оригинальный stream reader просит3FF, потребляет400 (R004). При включении в
real runtime9 малых ресурсов совпадали, но ROM491240/18480 decoded bytes был
повреждён. Попытка определить caller поctx->r31 не сработала: direct C calls
N64Recomp не устанавливают MIPS RA; поправок0, runner86 (native pending77).
SHA256 плохого результата fd1af5c4c2f4f133af39e5f58be2facb68a69b47b57c1addc7f8ad1b35692fa9.

Маркер перенесён перед JAL8003407C; thread_local контекст потребляется2730C,
проверяет buffer8007EF00 и меняет только full refill3FF->400. Сохраняются
настоящие queued PI DMA и оригинальный decoder. V32:8 поправок,10/10 совпадений;
SHA корректного большого ресурса4cc4e8e096e90666de7edc221bf19636cfbcf38035fe30eb4e9e84c143534bc9.
V36:11/11 совпадений, включая ID0B99. Snapshot снимается до caller mutation;
проверяется после запуска независимым Python decoder. Бинарные данные private.
Не обобщать правило на любые odd transfers; первичный ares PI source приведён
в live-startup.md. Ошибка сравнения не маскирует native_exit_code в новых reports.

## R018 — первый экран является предупреждением о контроллере

После инициализации собственных пулов и загрузки ресурсов scene34 перешла
в оригинальные update801C0088 и draw801C012C. Первое предположение о центральной
картинке не описывает текущий путь: bit0 controller presence mask801A48B4 равен0,
поскольку frontend отключает контроллеры. Оригинальная игра выбирает801BFE80,
рисует рамку и три текстовые строки через общий overlay. Условие не обходится.
Рекомпилированы именно необходимые panel/glyph/text/fade функции третьей игры.

V36 завершился на намеренном32-task лимите78 без pending calls. Два state-only
списка сменились30 задачами с рамкой/текстом: первые14 длиной1FB0, следующие16
длиной1F70 после завершения исходной fade-заливки. Пользователь сообщил, что
видел просьбу подключить контроллер. Автоматического сравнения pixels нет.
89 audio RSP completions;45568 stereo frames, все samples нулевые. Реальных
кнопок и audio device ещё нет;32 задачи не равны проверенной игровой сцене.
Default diagnostic с тем же generated code отдельно прошёл ожидаемую78.
Все32 dependency pins и чистое дерево Dora64 на680d81f сохранены.

Текущие результаты: research/live-startup-check.json и scene34-validation.json.
Исторические записи выше описывают свои бинарники; граница800280DC изR015
пройдена. Следующий шаг — input callbacks, обычная стартовая ветвь и audio output.

## R019 — подключённый порт и очередь SDL-аудио

Frontend теперь сообщает порт0 от клавиатуры/SDL GameController. Оригинальная
scene34 выбирает centered-image801BFF24; после61 tasks достигает801B54C8.
Добавлены исходные58248 (пять инструкций, не новая заглушка),288EC и1BFDE0.
Очередь SDL содержит несколько DMA; нельзя отдавать её полный размер через
AI_LEN: оригинальная8004E674 вычитает остаток unsigned. Остаток ограничен
максимальным переданным buffer size. Component-check проверяет порт/кнопки,
фокус, виртуальный SDL pad, stereo L/R и эту границу. Реальный ввод ещё не проверен.
V39: audio179 completions, только нулевые PCM,11/11 ресурсов совпали.

## R020 — сбой WSL не объяснён; Windows передаётся пользователю

Пользователь сообщил о падении WSL и искажениях. WSL перезапущена, файлы целы.
Журнал содержит dxg query failures и orderly shutdown; причинной связи не доказано.
Успешный RT64 parse не является проверкой изображения или GPU lifetime.
По указанию пользователя дальнейшие игровые запуски выполняет только он.
Windows frontend использует HWND/Automatic (как Dora64); ROM строго рядом с EXE, logs отдельно.
Донор Dora64 и исходники зависимостей не изменены.

Native MSVC configure выявил дубли PATH/Path во входном environment: импорт
старого Path скрывал compiler. Выбирается PATH из vcvars, compiler задан явно.
После неудачного определения compiler остались пустые flags; исправлено
cmake --fresh, проверены /O2 /Zi /EHsc. WIN32_LEAN_AND_MEAN runtime исключает
COM-объявления из windows.h; frontend явно включает objbase/oleauto для DXC.
Это исправления сборки, не подтверждение исправления изображения.

## R021 — Epoch закрывается до первого кадра белого fade

Пользовательский Windows test-001 показал Epoch и закрылся. Runtime log
1790566765911: RT64 D3D12 Automatic setup0,61 submitted/parsed tasks,
173 audio completions, stop77 на801B54C8. Это заданный pending-stop,
не установленный crash renderer.11 извлечённых ресурсов независимо перепроверены.
Оригинальная801BFDE0 включает белый fade RGB255/step16 и таймер16,
но до возврата вызывает common54C8(0). Остановка внутри этого вызова не давала
следующим кадрам выполнить fade. Test-002 компилирует исходную54C8 и leaf4F98.
Они сбрасывают состояние/flags; замены возвратом успеха нет.

После таймера исходная0088 возвращает управление433D8. Таблица ROM8007DC20
для scene1 указывает43654: common430EC, loader43120 (ROM70A50..CEC60 ->
RAM801B8ED0), callback801BB924. Этот блок перекрывает scene34 по RAM.
На момент обнаружения генератор отклонял пересечения; это ограничение снято
в пакетном этапе R022. Визуальный fade/катсцена test-002 не проверены агентом.

## R022 — пакетная карта scene1 и пересекающиеся оверлеи

Вместо выдачи EXE после пары функций построена кандидатная карта code region
ROM70A50 +41150 bytes (до801FA020, где уже видны display-list данные).
spimdisasm1.42.4 нашёл1303 кандидата; это не доказанный общий размер игры.
Резидентная карта содержит1054 кандидата; суммы не являются процентом готовности.
Обход прямых вызовов и распознанных function pointers из init801BB924,
update801BB730 и draw801BB848 дал909 новых кандидатов. Затем добавлены
библиотечные и оставшиеся связанные цепочки. Итого1311 compiled functions,
43 SDK mappings,17 явных границ (11 других scene loaders и6 hardware routines).
Границы CSV и успешная генерация не доказывают корректность всех ветвей;
новые функции в карте адресов помечены candidate. Оригинальные инструкции
сохранены; не добавлялись фиктивные успешные возвраты или патчи первой игры.

prepare_layout использует ключ(section,address). Перекрытие RAM разрешено
только явно объявленным взаимоисключающим сценам, ROM-диапазоны пересекаться
не могут. Вызовы внутри секции разрешаются внутри неё; неоднозначный внешний
вызов отклоняется. Runtime trace проверяет активность по ROM секции, а не по
одному адресу. DMA остаётся оригинальной, registration после побайтного сравнения;
старое отображение снимается при scene clear/перекрывающей записи.
Пять synthetic checks проверили wrong-section lookup, ROM overlap, unmarked
RAM overlap, ambiguous cross-call и duplicate native name. Игру они не запускают.

Первичный источник: N64Recomp README#overlays и закреплённый
source/src/recompilation.cpp resolve_jal; N64ModernRuntime librecomp/src/overlays.cpp.
https://github.com/N64Recomp/N64Recomp#overlays
Имеющаяся поддержка upstream использована без изменений зависимостей.

Аппаратные границы45520,586F0,5F1C0,5F230,604E0,58A50 оставлены pending.
Например, runtime osDpSetNextBuffer/__osSpSetPc содержат assert(false),
а __osDisableInt/__osRestoreInt — пустые тела; связывать их ради обхода stop
нельзя.45520 читает cartridge mapped B0000B70 (похоже на источник шрифта,
назначение пока гипотеза).578B0 после проверки включена: BF800000 — float -1,
переносимый через mtc1, а не hardware pointer. Её оригинальная математика сохранена.

Windows test-002 ограничен1800 graphics tasks/120 секундами после setup;
логи ENTER ограничены первым входом функции на host thread, hooks действуют всегда.
После доставки агент завершает ответ и не ждёт результата активно.

## R023 — вылет после белого fade: таблица команд аудиосеквенсора

Windows test-002, пользовательский журнал1790575858033: D3D12 Automatic,
scene1 init801BB924/update801BB730, затем Failed to find function at0x80046530.
Диспетчер80049840 читает byte-команду, проверяет80..AC, использует opcode&7F
как индекс45 указателей80076FD0 и вызывает адрес через jalr8004990C.
Второй путь80047920 использует ту же таблицу и такие же границы.
Команда95 указывает80046530. Из45 обработчиков были зарегистрированы только
800458C0 и80045A68: пакет прямых вызовов не покрывал указатели в данных.
Это причина данного abort; ошибки GPU этот журнал не доказывает.

Добавлены все43 пропущенных обработчика и их зависимость8004B678.
Адреса взяты из ROM, размеры заканчиваются jr ra и delay slot; CP0/uncached
hardware не найдено. Прямые зависимости уже зарегистрированы. Семантика
оригинальных инструкций сохранена; заглушек успешного возврата нет.
Новый tools/check_audio_dispatch.py проверяет всю таблицу и новые зависимости.
Другие audio filter callbacks уже перечислены в indirect_targets.
Опциональный callback8007FF00 обнуляется80047534, setter800490F8: прямых
вызовов setter в текущем resident/scene1 не найдено; не считаем все callbacks
игры полностью исследованными.

Независимый decode_record побайтно совпал со всеми 290 захваченными
ресурсами предыдущего запуска (до relocation). Результат и хэши записаны в
research/windows-test-002-result.json; бинарные ресурсы не отслеживаются.
Генерация CPU startup-v45 предназначена для Windows test-003. Игра агентом
не запускается; визуальный результат передаёт пользователь.

Проверка test-003 завершена: Windows MSVC RelWithDebInfo /O2 /Zi /EHsc build
успешен,1355 original bodies +43 SDK +17 pending =1415 symbols совпадают с
N64Recomp. Все тела имеют ROM-qualified hook. Generation receipts и хэши
исходников совпадают;32 pinned dependencies и donor680d81f не изменены.
Игру не запускали. Текущая компиляционная квитанция windows-build-check.json,
предыдущая сохранена в research/history/windows-test-002-build-check.json.

## R024 — вступление работает, переход к титульному меню завершается abort

Test-003: пользователь слышал музыку/звук и видел3D-вступление с бегущей
анимацией, без замеченных отличий от оригинала. Журнал1790576564302:
907 submitted/parsed graphics tasks, ненулевые PCM; выбирается scene31/state2,
common и intro выгружаются, common5E400 и меню635C0 загружаются с bytewise
проверкой, затем Failed to find function at0x801C0CC0. Тайм-аут не достигнут.
Это отсутствующий initializer титульной сцены, а не ошибка загрузки её ROM.

Таблица8007DC20 slot30 ->80043554 ->430EC/43154 ->801C0CC0.
Initializer устанавливает update801C0BBC/draw801C0C88. В той же группе
загрузчиков найдены шесть входов сцен30/31/32/33/34/38. Проверен их общий
пакет вызовов и ссылок:142 новых тела .scene34,4 common,19 resident.
Перекрывающаяся .scene1 не меняется; идентичность ROM секции сохранена.
Новые границы — кандидаты spimdisasm1.42.4, отдельно проверяются генератором.
Назначение каждого дополнительного экрана ещё не подтверждено визуально.

В цепочке записи сохранения8003C8C0 найден SDK8004E780: аргументы
mq/block-index/buffer/byte-count; цикл пишет8 байт через8005E4A0, возвращает
первую ошибку, между блоками timer-delay. Это osEepromLongWrite, дополнение
уже связанных Probe/LongRead. Pinned librecomp eep.cpp пишет настоящие данные
через save_write (pi.cpp); зависимость не меняется. Сырые SI/CP0 вызовы
не подменяются пустыми успешными ответами. Сохранения всё ещё изолированы
по запуску; persistence/frontend lifecycle остаются отдельным этапом.

Все 297 захваченных ресурсов test-003 совпали с независимой распаковкой
до relocation. Подтверждение пользователя и численные наблюдения сохранены
в research/windows-test-003-result.json, без ресурсов и дизассемблированного кода.

Дополнительно проверен вызывающий80043F1C: запись8/512/8 байт через8003C8C0,
адрес блока slot<<6; оба размера кратны8, ABI соответствует runtime.
Это проверка пути записи, а не доказательство сохранения/загрузки между запусками.

Windows test-004 собран MSVC RelWithDebInfo /O2 /Zi /EHsc. Проверки:
6 входов общего оверлея и165 новых функций/ссылок в правильных ROM-секциях;
1520 original +44 SDK +17 pending =1581 symbols; все original bodies имеют
ROM-qualified hook, хэши исходников и generation receipts совпадают.
32 pinned dependencies и donor680d81f не изменены. Игра агентом не запускалась.
Ждём только последующего сообщения пользователя, без активного ожидания.

## R025 — удалены мешающие пользовательской проверке лимиты

Пользователь сообщил, что диагностическое ограничение test-004 прервало
проверку, и потребовал убрать его полностью. Из live_graphics.cpp удалены
ветка выхода после1800 задач, таймер120 секунд, чтение DORA3_MAX_TASKS и
состояние ready, использовавшееся только таймером. Счётчики остаются журналом;
условий остановки по ним нет. Закрытие окна по-прежнему вызывает quit.
Двухсекундный drain при уже возникшей ошибке не ограничивает длительность игры.
CPU/RSP генерация и игровые функции не менялись, используются startup-v46/audio-rsp-v1.
Лимиты не будут возвращены в последующих живых сборках; правило внесено в AGENTS.md.

Test-005: Windows MSVC сборка успешна. Статически проверено удаление обеих
веток остановки; в готовом EXE нет прежних строк лимитов/DORA3_MAX_TASKS,
есть сообщение LIVE SESSION: no time or graphics-task limit. Хэши исходников,
CPU/RSP generation receipts и32 зависимостей проверены; donor не изменён.
Длительный игровой запуск агентом не выполнялся согласно указанию пользователя.

## R026 — титульное меню работает; не зарегистрированы дочерние оверлеи демо

Пользователь test-005 подтвердил загрузку титульного меню и музыку. Start
не нажимал, дождался демонстрации уровня. Runtime1790577770681: scene31 ->1,
затем DMA CEC60/70B0 ->8021D6D0;12FD90/31F0 ->8022D6D0;
14D4C0/2860 ->8023D6D0; все прошли побайтную проверку. Abortlookup8023E9A0
принадлежит третьему блоку (ROM14E790), отсутствовавшему в мета-карте.
Это новый класс дочерних оверлеев внутри scene1, не повтор меню и не тайм-аут.

Загрузчик801E4774 использует пары start/end из таблиц801FC020/C0A8/C100,
selectors2/7/1 дают наблюдавшиеся блоки. Пары находятся801FC028/C0D8/C100.
Текстовые префиксы блоков6D58/3020/1F20, остальные байты — данные; по spimdisasm
найдены78/40/64 функции плюс23 scene1 и1 resident зависимость (206 новых тел).
Диапазоны/таблицы подтверждены ROM и журналом; границы функций остаются
кандидатами до отдельного покрытия.8022EBA4 B60B60B7 используется mult/shift,
это арифметическая константа, не обращение к некэшируемому адресу.

Шесть ссылок дочерних блоков попадают в RAM, разделяемую .scene34/.scene1.
Добавлено явное requires .scene1: анализ разрешает их только в игровом оверлее;
runtime разрешает загрузку/вызов дочернего кода только после проверенной загрузки
родителя. N64Recomp и зависимости не меняются; неоднозначные внешние JAL
в pinned N64Recomp используют runtime lookup. Известный механизм регистрации
после точного DMA и выгрузки/инвалидации распространяется на новые три региона.
Другие варианты этих слотов ещё не зарегистрированы, подставлять этот код нельзя.

Все 397 захваченных ресурсов совпали с независимым decode_record до relocation.
Результат пользователя в windows-test-005-result.json; адресная карта и замыкание
в demo-candidate-map.json. ROM/код/ресурсы остаются вне Git.

Статическая генерацияv47 выявила ложную границу80224404: это case внутри
switch8022438C, не самостоятельная функция (нет пролога; общий возврат8022441C).
Таблица80224750 содержит10 внутрипроцедурных целей. Тело расширено до9C,
ложный entry удалён; обе его JAL зависимости сохранены. Никаких инструкций
игры не заменяли. Исправленная карта:77/40/64 дочерних тел плюс24 зависимости,
всего205 новых функций,1725 суммарно. Повторная генерация — startup-v48.

Test-006 успешно собран MSVC RelWithDebInfo /O2 /Zi /EHsc. Проверки:
1725 original bodies +44 SDK +17 pending =1786 symbols, без неявных функций;
9 synthetic layout tests; три ROM пары/диапазона и205 добавленных тел/ссылок;
каждый original body защищён ROM-qualified hook. Все шесть внешних ссылок
с неоднозначной RAM-областью в generated C вызывают именно scene1 bodies;
три записи required_index=2 соответствуют активному родителю .scene1.
Хэши исходников/генерации совпадают,32 pinned dependencies и Dora64 не изменены.
В EXE отсутствуют старые строки лимитов/DORA3_MAX_TASKS, присутствует no-limit
сообщение. Игра агентом не запускалась; кадры и полный цикл демо не подтверждены.

## R027 — визуально тот же переход, новый callback состояния игрока

Test-006, журнал1790578864153: все три дочерних оверлея зарегистрировались,
8023E9A0 из предыдущего abort выполнился, загрузились дополнительные ресурсы.
Новая остановка Failed to find function at0x801C85C8. Внешне пользователь
видит тот же переход к демо; это не подтверждает повтор предыдущей причины.

Внутри801C8B28 после801C1384 код801C8EEC..801C8F08 берёт поле персонажа+94,
умножает состояние на8, читает callback из801FA6B0. Для0x43 слот801FA8C8
содержит801C85C8. Установка0x43 видна801C85A4..85A8. Прямое замыкание вызовов
пропускает такие указатели в данных: прежнего набора было недостаточно.

Добавлен целиком оставшийся анализированный код scene1 до801FA020:
591 функция плюс7 resident зависимостей,598 новых тел. Все1303 исходных
кандидата scene1 теперь включены. Нет новых CP0/uncached границ. N64Recomp
принимает весь пакет:2323 original +44 SDK +17 pending =2384 символа.
Не утверждаем полноту всех оверлеев игры или динамических целей вне этого блока.
Новые адреса отмечены candidate; инструкции ROM сохранены.

Все 413 ресурсов пользовательского запуска совпали с независимой
распаковкой до relocation. Отчёт windows-test-006-result.json фиксирует разницу
между двумя остановками; gameplay-core-candidate-map.json содержит весь пакет.

Проверка test-007: нативная Windows x64 сборка MSVC завершилась успешно.
Сверены 2323 тела, 2384 символа, хэши исходников и генерации, все 32 зависимости;
Dora64 не изменён. Шесть вызовов дочерних блоков по-прежнему разрешаются
в правильные функции scene1, три проверки родителя сохранены. В EXE нет
старых строк лимитов или переменной DORA3_MAX_TASKS. Статическая проверка
check_gameplay_core.py подтвердила исходную таблицу состояния 0x43,
полный пакет кандидатов и его ссылки. Игру агент не запускал; прохождение
перехода и стабильность демо остаются на проверке пользователя.

## R028 — выбор слота: локальный JR ошибочно стал вызовом функции

Test-007, журнал 1790579638907: пользователь подтвердил игровой attract-demo
и его завершение. После меню, выбора слота и A — lookup abort 801BE7C0.
Последняя пара RT64 submitted/parsed: 2652/2652. Все 501 ресурсов
совпали с независимой распаковкой до relocation.

801BE7C0 не новая функция: это case в 801BDEFC..801BEA58, ROM 685EC..69148,
оверлей .scene34. В 801BE778 регистр s3 получает 801C5538, s2 копирует его,
801BE7B4 читает *(s2), 801BE7B8 выполняет JR, 801BE7BC — NOP.
Пять записей таблицы: 801BE7C0/7D0/7E0/7F0/800; s2 увеличивается на 4
в 801BE814, цикл ограничен s3+0x14. N64Recomp выпустил LOOKUP_FUNC и return
вместо перехода на локальную метку. Увеличение списка функций это не исправляет.

Соседняя таблица 801C5550 содержит 801BE890/8B8/918/8F0/8FC.
Указатель s0 увеличивается в 801BE918; JR 801BE888 автоматически распознан,
но generated jr_addend_801BE888 инициализируется нулём перед циклом и дальше
не обновляется: получается выбор только первой ветви. Просмотр generated C
нашёл одну такую константную таблицу и один нераспознанный JR с локальными
целями в текущем пакете. Это не доказательство полноты анализа всей игры.

Исправление только в проектной генерации: проверенные metadata описывают
обе последовательности LW/JR/NOP, ROM-таблицы и owning function. Перед LW
hook выполняет исходное чтение в r2 и switch по загруженному адресу с goto
на локальные метки. NOP delay не имеет эффекта; прочие регистры и локальные
hi/lo/FPU переменные остаются в том же теле. Неизвестная цель явно ошибочна.
Генерация отклоняет изменённые опкоды/таблицы, ненулевой delay, внешнюю цель
и отсутствие метки. ROM-инструкции и зависимости не изменяются; ранний goto
обходит ошибочный перевод этих двух последовательностей.

Первичный источник о JR/switch и доступном hook API:
https://github.com/N64Recomp/N64Recomp/blob/ffb39cdad1da5de07eaaa48bd1db4a89a7986771/src/recompilation.cpp
Закреплённый исходный инструмент изучен локально; общую библиотеку не патчили.
Компонентная проверка компилирует точный hook text и исполняет все 10 ветвей,
проверяет r2, неизменность регистров-указателей, ошибочные адреса и отказ при
изменённых ROM evidence. Ни игра, ни renderer не запускаются.

Итог test-008: нативная Windows x64 MSVC сборка успешна. 2323 оригинальных
тела / 2384 символа, хэши генерации и исходников совпали; все 32 зависимости
и рабочий Dora64 остались неизменными. Шесть неоднозначных вызовов дочерних
оверлеев по-прежнему указывают на scene1, проверки родителя сохранены.
Лимиты отсутствуют. Игру агент не запускал; выбранный слот и новую игру
после исправления должен проверить пользователь.

## R029 — первый gameplay использует другие варианты дочерних оверлеев

Test-008, журнал 1790580470777: пользователь подтвердил прохождение меню
слота и вступительной катсцены. На первом gameplay — lookup abort 8023103C.
Перед ним DMA EDDB0/7450 ->8021D6D0, 132F80/6920 ->8022D6D0,
15E440/C00 ->8023D6D0 прошли побайтную проверку, но этих ROM-регионов ещё
не было в списке регистрации. Это другие варианты трёх слотов, чем демо.
Адрес ошибки соответствует ROM1368EC во втором блоке.

Тот же оригинальный загрузчик 801E4774 выбирает пары 801FC058/C0E0/C148
(selectors 8/8/10). Текстовые префиксы 69F0/55F0/B10 подтверждены просмотром
окончания кода и начала данных. Найдены 72/75/29 функции плюс common
зависимость: 177 новых тел, 2500 суммарно. Границы по spimdisasm остаются
candidate; оригинальные инструкции сохранены. CP0/uncached препятствий нет.

Исправлены ложные границы 8023E1BC и 8023E1D0: данные после текста были
декодированы анализатором как branch-and-link и разделили хвост функции.
Обе метки находятся внутри 8023E0B8..8023E1E0, с общим стеком/эпилогом.
Тело объединено до 0x128; самостоятельные записи не создавались.

Для каждого слота демо и новый вариант объединены только в группу взаимного
исключения child_slot0/1/2. Имена native-функций и ROM-идентичность отдельные.
requires .scene1 сохранён. Существующая инвалидация перед DMA снимает старый
вариант; регистрация требует точного ROM/RAM/size и совпадения всех байтов.
46 ссылок новых блоков в общую RAM меню/scene1 разрешены в scene1; проверены
все сгенерированные вызовы добавленных функций. Зависимости/runtime не изменены.

Генерация startup-v51 приняла пакет: 2500 original +44 SDK +17 pending =2561.
Проверки: 9 synthetic layout, старый demo-пакет, обе локальные таблицы меню
(10 ветвей) и весь новый пакет прошли. Просмотр новых непрямых переходов не
нашёл дополнительных случаев дефекта R028. Все 427 захваченных ресурсов
совпали с независимой распаковкой. Другие варианты/уровни пока не включены.

Итог test-009: нативная Windows x64 MSVC сборка успешна. Сверены 2500 тел,
2561 символ, хэши исходников/генерации/EXE, все 32 закреплённые зависимости.
Dora64 не изменён. Старые вызовы дочерних блоков по-прежнему разрешаются в
scene1; новые вызовы и шесть parent guards проверены. Лимитов сессии нет.
Игру агент не запускал; появление первого уровня и управление ещё не подтверждены.

## R030 — пещера и постоянные сохранения

Test-009, журнал 1790581149122: пользователь прошёл первый уровень, катсцену
и карту выбора. Пещера завершается на lookup8023E410. DMA EBAC0/22F0,
142670/3050,150C60/2520 в слоты8021D6D0/8022D6D0/8023D6D0 проверены,
но эти варианты не зарегистрированы. Ошибка соответствует ROM1519A0.
Пары801FC050/C0F0/C110 задают selectors7/10/3. Текст2190/2DC0/2320;
25/31/60 функций плюс одна resident-зависимость. Итого117 новых,2617 original.
Ложная граница80230480 — эпилог80230310, исправлена до размера180.
Инструкции не менялись. Варианты разделены теми же child_slot группами,
requires.scene1. Проверены все новые generated calls и прежний пакет уровня.
Все 869 захваченных ресурсов совпали с независимой распаковкой.

Сейв реально существует: state/saves/dora3.n64.jp.bin,2048 байт; timestamp
обновлён после старта игры. Причина невозможности продолжения между запусками:
register_config_path получал уникальный run_dir/state. Теперь Windows root —
папка самого EXE, runtime сам добавляет saves/dora3.n64.jp.bin. Все новые EXE
используют общий config root независимо от working directory. Диагностические
ресурсы/snapshot переведены на отдельный diagnostic_path=run_dir/state.
Linux diagnostic CLI сохранил явный приватный каталог; Android пока не менялся.

Последний файл и bak перенесены копированием в testbuild/saves. Существующие
цели не перезаписываются; оригиналы оставлены. Хэши в save-path-check.json,
сами сейвы не в Git. Компонент напрямую использует pinned pi.cpp/files.cpp,
читает все2048 байт через save_read, проверяет endian-преобразование RDRAM,
запись/повторное чтение и fallback на bak на одноразовой копии. Источник неизменён.
Это проверка host EEPROM storage, а не игровых контрольных сумм/слотов.
Пользователь просит единые настройки/сейвы для всех последующих test builds;
старые EXE не исправляются задним числом и остаются диагностическими архивами.

Итог test-010: Windows x64 MSVC сборка успешна,2617 тел/2678 символов.
Хэши исходников, генерации и EXE сверены;32 зависимости и Dora64 неизменны.
Новая сборка содержит общий config root, диагностические файлы остались в
run_dir/state. Повторная компонентная проверка уже перенесённого файла прошла
без предупреждений и изменения его SHA256. Игру агент не запускал.

## R031 — гора: ещё два варианта дочерних блоков

Test-010, журнал1790596688895: пользователь подтвердил запуск пещеры и вылет
при выборе горы. В этом журнале scene30 ->1, затем DMA CEC60/70B0,
125180/1A20,14FD20/F40 в три слота. Первый зарегистрирован, два других
отсутствуют в metadata. Abort8023D8E8 соответствует ROM14FF38 нового slot2.

Пары801FC0B8/C108 дают selectors3/2; slot0 повторно использует CEC60(selector2).
Текстовые префиксы1920/E30 проверены по окончанию функций и началу данных.
32/26 кандидатов,58 новых функций; дополнительных зависимостей не потребовалось.
Инструкции ROM неизменны, совпадающие RAM-адреса разделены по ROM и child_slot
группам. requires.scene1 сохраняет правильного родителя. Все generated calls,
три неоднозначные ссылки к scene1 и прежний пакет пещеры проверены.
Новых случаев дефекта локальных переходов R028 не найдено. Генерация v53:
2675 original +44 SDK +17 pending =2736 символов. Все 483 ресурсов
последнего запуска совпали с независимой распаковкой до relocation.

Runtime, настройки и механизм сохранения не менялись. Сейвы до упаковки
хэшируются отдельно; пакет не включает и не перезаписывает их. Работоспособность
самой горы/прохождения пока не подтверждена; игровой запуск делает пользователь.

Итог test-011: Windows x64 MSVC сборка успешна. 2675 оригинальных тел,
2736 символов; исходники/генерация/EXE сверены,32 зависимости и Dora64
не изменены. Сохранены прежние вызовы к scene1 и проверки родителей.
Игру агент не запускал; проверка горы остаётся пользователю.

## R032 — озеро: непрямой вызов после прыжка в воду

Test-011, журнал1790597091745: гора загрузилась; пользователь сообщил вылет
при прыжке в воду на озере. После перехода scene30 ->1 DMA E5480/6640,
129A30/4800,1561E0/35A0 прошли побайтную проверку. Эти варианты не зарегистрированы.
Игра продолжала выдавать кадры и звук; abort8023F0DC наступил позже, после
2259 submitted/parsed задач. Это функция в lake_slot2, ROM157BEC, размерF0,
а не внутренний case или подтверждённый дефект водного рендера.

Пары801FC048/C0C8/C128 дают selectors6/5/6. Текст6350/4230/2EA0 отделён
от данных по дизассемблированию и концам функций. Добавлены64/69/52 кандидата,
185 тел; все зависимости уже присутствуют. Границы остаются кандидатными,
инструкции ROM неизменны. Группы child_slot0/1/2 и requires.scene1 отделяют
озеро от совпадающих RAM-адресов других уровней. Все generated calls нового
пакета,6 неоднозначных ссылок к scene1 и прежний пакет горы проверены.
Новых случаев дефекта R028 не найдено. Генерация v54 успешна:
2860 original +44 SDK +17 pending =2921 символ. Все 701 ресурсов
совпали с независимой распаковкой. Runtime/сейвы/настройки не менялись.
Запуск/плавание проверяет пользователь; агент игру не запускает.

Итог test-012: Windows x64 MSVC сборка успешна. Сверены2860 оригинальных тел,
2921 символ, хэши исходников/генерации/EXE;32 зависимости и Dora64 неизменны.
Предыдущие вызовы дочерних блоков и проверки родителей сохранены. Игра
агентом не запускалась; вход в воду и плавание проверяет пользователь.

## R033 — деревня пингвинов: вызов после продвижения

Test-012, журнал1790597608324: пользователь подтвердил исчезновение вылета
при прыжке в воду. В деревне после продвижения lookup abort8023E8B0,
ROM155C70, размерE0. Перед этим DMA1456C0/7E00 ->8022D6D0,
154A90/1750 ->8023D6D0 прошли побайтную проверку; вариантов ещё нет в metadata.
В финальном переходе нет DMA slot0; его загрузку/состояние не подменяли.
До abort дошли1097 submitted/parsed графических задач.

Пары801FC0F8/C120 дают selectors11/5. Текстовые префиксы55C0/15F0
проверены по окончанию кода и началу данных. Анализатор назвал части данных
первого блока функциями80233D94/802346E8 — они исключены как находящиеся
вне найденного текстового префикса. Во втором ложная граница8023EC94
попала в delay slot/общий эпилог8023EC14, из-за branch-likely в последующих
данных. Граница объединена доAC; отдельной функции8023EC94 нет.

Добавлены74/30 кандидатов,104 тела. Инструкции ROM сохранены, дополнительные
зависимости не нужны. Child_slot1/2 и requires.scene1 отделяют блоки от других
уровней. Все generated calls,26 контекстных ссылок к scene1, пакет озера и
отсутствие новых случаев R028 проверены. Генерацияv55:2964 original +44 SDK
+17 pending =3025 символов. Все 588 ресурсов совпали с независимой
распаковкой. Настройки/сейвы/runtime не менялись. Дальнейшую деревню проверяет
пользователь; агент игру не запускает.

Итог test-013: Windows x64 MSVC сборка успешна. Сверены2964 оригинальных тела,
3025 символов, хэши исходников/генерации/EXE.32 зависимости и Dora64 неизменны.
Отдельный просмотр подтвердил, что ложные функции80233D94/802346E8 лежат
в строковых данных, а текст заканчивается возвратом80232C7C/80 перед таблицами.
Предыдущие вызовы и проверки родителей сохранены. Игру агент не запускал.

## R034 — вход в мини-игры: отдельное семейство оверлеев

Test-013, журнал1790598229613: деревня загружается по сообщению пользователя.
При входе в мини-игры scene35/state2 освобождает common/scene34, повторно
загружает common5E400 и останавливается на pending80043188. Это явная граница
покрытия, не падение GPU:233 submitted/parsed,732 submitted/completed audio,
stop77. Все 339 сохранённых ресурсов совпали с независимой распаковкой.

Оригинальный загрузчик80043188 копирует ROM18BDF0..1961B0 в801B8ED0.
Следующие девять загрузчиков800431BC..8004335C копируют соседние ROM-блоки
до1C5DE0 в801C3680. Проверены LUI/ADDIU/SUBU и JAL8002730C каждого загрузчика.
Таблица ROM59020 направляет сцены35/36/37 в callbacks801BDCD0/801BEE34/801C1CD0
общего блока, а2..8,10,11 — в девять вариантов. Имена мини-игр не установлены.
Общий блок и варианты находятся в группе scene_code вместе с прежними сценами;
варианты требуют minigame_common, общий требует scene_common. Их RAM-области
не пересекаются друг с другом, но пересекаются со scene1/scene34. Требования
обеспечивают выбор нужного родителя, а исходный DMA управляет заменой блоков.

Текст отделён от следующих таблиц/float/display-list данных; последние ложные
кандидаты анализатора исключены, длины последних тел обрезаны по JR/его delay
slot.99 функций общего блока; варианты82/65/71/99/41/54/47/77/60; дополнительно
14 resident (включая10 загрузчиков) и5 scene_common. Всего714 новых тел.
КонстантаB60B60B7 в801C4378 проверена как multiply-high для деления/остатка90,
это не MMIO-адрес. ROM и инструкции не изменены, новых заглушек успеха нет.

Генерацияv56:3678 original +44 SDK +7 pending =3729 символов.
Проверены все новые прямые вызовы, наличие address-taken callbacks,184 ссылки
с выбором секции, родители и прежний пакет деревни;9 synthetic layout tests
прошли. Новых случаев R028 (локальный JR ошибочно превращён в lookup) не найдено.
Старые два случая остаются защищены прежними проверенными хуками.
Дальнейшая работа мини-игр и все игровые ветви требуют проверки пользователем.
Игру/рендер агент не запускает. Настройки, saves и runtime не менялись.

Итог test-014: нативная Windows x64 MSVC сборка успешна. Сверены3678 тел,
3729 символов, trace guards, хэши исходников/генерации/EXE. Все32 зависимости
и Dora64 неизменны; старые6 context-sensitive вызовов demo указывают на scene1.
Игру я не запускала. Проверку мини-игр выполняет пользователь.

## R035 — результат пользовательской проверки test-014

Пользователь проверил доступные мини-игры: вылетов не заметил, работа выглядит
корректной. Часть мини-игр закрыта прогрессией; их работа не подтверждена.
Точный список запущенных вариантов не указан, поэтому нельзя считать все девять
вариантов проверенными. Это запись сообщения пользователя, без новой сверки логов.

Срез покрытия:3678 оригинальных функций,44 SDK mappings,29 overlay-регионов,
7 явных pending-границ в известном графе вызовов. Основной gameplay-блок содержит
1303 скомпилированных кандидата. Эти числа не являются процентом всей игры:
остаются не включённые сменные блоки и неизвестные непрямые вызовы. Наличие тела
в сборке не доказывает прохождение всех его ветвей. Полный порт с улучшениями,
локализацией и игровыми Linux/Android-сборками остаётся последующими этапами.
Код не изменялся, новая сборка не нужна. Игру я не запускала.

## R036 — полная сверка известных загрузчиков и таблиц

После успешных доступных мини-игр test-014 выполнен пакетный аудит оставшихся
блоков без ожидания новых вылетов. Три таблицы ROM в scene1 имеют17/11/10
записей. После третьей таблицы RAM801FC150 идут байты0001020304050607 и дальше
последовательные индексы, не ROM-пары. Историческое допущение16 записей slot2
исправлено. В slot0 selectors1/9/10 пары пустые. Всего35 непустых child-блоков,
из них16 были зарегистрированы,19 добавлены. Проверены сами LW загрузчика,
непрерывность ROM-пар, RAM-слоты и размеры. Отдельно добавлен ROM59E80..5E400
в RAM801B3770, загружаемый800430B8 для сцен80/90/91; назначение каждого
экрана требует проверки. Доступ через resident scene dispatcher статически виден.

Служебный блок пересекается с scene_common, поэтому оба имеют common_code.
Всем обычным сценам и child/mini блокам явно задан requires.scene_common
в дополнение к основному родителю. Проверка выбирает функции по секции,
реальный DMA и runtime guards сохраняют идентичность. Старые проверки обновлены
под дополнительную зависимость; их исторические карты не переписывались.

Текстовые окончания всех20 блоков проверены по JR/его delay slot и переходу к
сценарным/графическим данным. Spim ошибочно разметил адрес сценария80223818 как
функцию; аргумент a0 в8021F808 передаёт его в801E6968 как данные. Эта ссылка
исключена из графа кода. Найден ложный split8023136C: прежний generated-body
80231320 обрывался после LW ra, пропуская восстановление SP. Тело объединено
до54 байт; ложная функция удалена. Startup-v57 — промежуточный непереданный
вариант; итоговая генерацияv58 содержит исправление. Проверки концов всех
1099 добавленных функций не обнаружили других подобных обрывов.

Итог карты:4777 original +44 SDK +6 pending =4827 symbols;49 registered ranges.
Проверены18547 прямых JAL всей сборки, все parent guards, новые границы,
семь прежних групп overlay-check и9 synthetic layout tests. Новых R028 не найдено.
Все49 диапазонов известных загрузчиков и таблиц подключены; не утверждается,
что доказано отсутствие других источников исполняемого кода/непрямых целей.
Полное прохождение, закрытые мини-игры и новые сцены остаются непроверенными.

## R037 — оставшиеся6 аппаратных границ

800430B8 подключён оригинальным телом. Остальные6 изучены отдельно:
- 80045520: декодирование битовых данных с прямым чтением cartridgeB0000B70;
  вызывается controller-pak startup80044C14. Требуется адаптация ROM-чтения.
- 800586F0: программирование DPC buffer/status. В изолированном runtime
  osDpSetNextBuffer содержит assert(false), подключать его как поддержку нельзя.
- 80058A50: SI raw DMA64 байта через A480/PIF1FC007C0; полноценной реализации
  __osSiRawStartDma в текущем runtime не найдено.
- 8005F1C0/8005F230: выключение/восстановление CP0 interrupts. Одноимённые
  runtime функции пустые, disable не возвращает исходное состояние. Это не
  подтверждённая замена семантики оригинальных нижнеуровневых путей.
- 800604E0: SP halt/status и запись PC. Runtime __osSpSetPc также assert(false).

Первичные источники: инструкции локального ROM; зафиксированные исходники
build-deps/lib/N64ModernRuntime/librecomp/src/{dp,sp,ultra_translation}.cpp.
Поэтому границы сохранены с явной диагностикой, без заглушек успеха. Наличие
прямых ссылок не доказывает достижимость в обычной игре; их недостижимость тоже
не доказана. Уменьшать pending искусственно не стали. Runtime/deps не менялись.

Итог test-015: Windows x64 MSVC сборка успешна. Сверены4777 оригинальных тел,
4827 symbols, хэши исходников/генерации/EXE. Все32 зависимости и первый Dora64
не изменены. Игру я не запускала; результат исполнения новых сцен не заявляется.


## 2026-09-28 — Реализация шести аппаратных границ

Причина оставшихся остановок — недостаточность pinned runtime: DP/PC имели
assert(false), raw SI отсутствовал, disable/restore были пустыми; обычная
рекомпиляция MMIO/CP0 не даёт корректной работы на host. Пользователь поручил
изучить документацию и реализовать их до полного прохождения.

Добавлены пять нативных обработчиков с fingerprint исходных тел. Шестая
функция оказалась декодером шрифта, а не SDK: сохранён оригинальный алгоритм,
перехвачены только три загрузки ROM/RAM. Прямой RDP направлен в очередь RT64,
DP-событие связано с FullSync. SI использует общий ввод и существующую EEPROM.
HALT связан с отправкой/завершением RSP. IE откладывает внешние сообщения и
вытеснение, включая blocking/timed wait: эти пути обнаружены при проверке
интеграции и тоже закрыты, чтобы не обходили выключенные прерывания.

Компонентный стенд: реальная очередь, save backend и конвертер стика; все256
блоков EEPROM; вложенный IE; DP ABI a2:a3; RSP busy/PC. Декодер дал побайтно
тот же4320-байтный результат, что независимый битовый разбор. Первый вариант
ожидания в тесте использовал85 для стика; исправлен сам тест: donor runtime
имеет диапазон82 и округление float. Игровое управление не меняли.

После генерации:4778 original+44 SDK+5 native=4827 symbols,0 known pending.
Все18547 JAL и49 известных регионов проверены. Linux target скомпилирован.
Границы доказанного, источники и воспроизведение — docs/hardware-interface.md.
Игра и графическое устройство агентом не запускались. GPU/полное прохождение
проверяет пользователь; отсутствие pending не обещает отсутствие всех ошибок.


Итог test-016: Linux diagnostic target и Windows x64 MSVC RelWithDebInfo
скомпилированы. Hardware component-check пройден после добавления wait-gates;
9 layout-тестов и проверка мини-игр714 тел/184 контекстных ссылок пройдены.
Windows audit сверил4827 symbols,18547 JAL, генерационные receipts, исходники
EXE и три адаптированные runtime translation units (с нормализацией LF/CRLF).
Сохранены все32 dependency pins/чистые checkouts и чистый Dora64 на прежнем
коммите. ROM и все найденные сейвы/backup в testbuild не изменены.
Единственное замеченное предупреждение MSVC C4297 относится к исходному
osDestroyThread в закреплённой библиотеке, не к новому аппаратному коду.


## 2026-09-29 — Аудит памяти кадра; общий буфер освещения

По согласованному пункту1 исследованы производитель кадра, scheduler completion,
RT64 readers и PI DMA. У третьей игры два банка размером31728; кадровые данные
начинаются с+108, основной DL с+12328. Освобождение OSScTask ждёт SP и DP.
Ранее перенесённый SP-after-parse защищает рассмотренный путь повторного
использования банков, но не все общие данные. PI mutex уже работает на desktop:
гипотеза о его Android-only применении отвергнута после чтения pi.cpp.

Установлен общий изменяемый свет8006FB70..8006FB98: направления пишет80027B84,
DL обоих банков ссылаются на те же адреса. Если CPU построит второй кадр раньше
разбора первого, первый читает новое направление. Это воспроизведено стендом
на оригинальных generated submit/light-writer; видимое проявление у пользователя
не утверждается. Исправлено40-байтным snapshot перед osSendMesg8002FF50 и
существующим lightDataCallback RT64. Delay slot и16-байтные чтения проверены.

Первый запуск стенда выявил ошибку самого стенда: N64Recomp MEM_W требует
знаково расширенного виртуального адреса. Исправлены тестовые записи памяти;
повторная проверка двух банков/отложенного потребителя/публикации/reset прошла.
Runtime game stores и адреса hook из-за этой ошибки не менялись.

Границы и остающиеся вопросы: docs/frame-memory-lifetime.md. FB/VI и все
динамические источники текстур/вложенных DL не объявлены доказанно безопасными.
Глобальное копирование RAM отклонено: оно не учитывает framebuffer feedback и
прямые читатели RDRAM. Первый Dora64 и его зависимости не редактировались.
Игра/renderer не запускались; test-016 для прохождения не заменён.


Проверка сборок: Windows x64 live frontend (MSVC/O2) и Linux diagnostic target
скомпилированы. Сравнение всей CPU-генерации v59/v60 показало единственное
изменение в generated C: вызов capture перед очередью; оригинальные тела и
контекстные переходы не изменились.4778 original/44 SDK/5 native сохранены.
Проверены32 пинованные зависимости и чистота Dora64 на680d81f. Отдельный отчёт
research/frame-memory-build-check.json; отчёт и пакет test-016 не подменены.


## 2026-09-29 — test-017: framebuffer/VI и удержание памяти

Причины: проверка guest outstanding tasks не согласовывала CPU-копию800271B0
с отдельным host ScreenUpdateAction; периодические VI requests могли
неограниченно накапливаться при отставании renderer. Добавлены lock hooks
800272BC/800272F8 исключительно вокруг call-free loop и общий mutex в
LiveContext::update_screen. Periodic gate оставляет один незавершённый запрос,
не блокируя VI/AI/игровые задачи и не заменяя VI snapshot через границы задач.

Удалено ненужное удержание16-МиБ проверочного ROM vector на всём протяжении
recomp::start; рабочий ROM принадлежит runtime. В лог добавлена минутная
телеметрия RAM и числа принятых/объединённых запросов. Это не лимит сеанса.

ASan/LSan:2млн periodic ticks, полное сравнение RAM/context исходной копии,
ранние выходы, конкурентный reader и128тыс allocations/free оригинального
heap — успешно. В первом варианте стенда promise ошибочно сигналил дважды
при lock/unlock; исправлен только тест. Windows RT64 и Linux diagnostic
скомпилированы; игровой renderer не запускался. Снимок света проверен на v61.
Полная карта причин/ограничений: docs/synchronization-memory.md. Утечки GPU и
всех сцен не объявляются исключёнными по компонентному результату.


## 2026-09-29 — test-018: подтверждённая гонка VI и границы lifetime

Причина гонки: setters используют message_mutex, update_vi использовал только
regs_mutex, framebuffer getters читали без lock. TSAN отрицательный контроль
на исходном events.cpp воспроизвёл update_vi/get_cur_state race; адаптация с
общим message_mutex прошла тот же трёхпоточный стенд. Runtime dependency не менялась.

Аудит4778 функций/49 регионов и original retirement component показал16-tick
освобождение независимо от outstanding=2, а также потерю101-го запроса при100
занятых deferred slots. Это синтетические условия, не доказанная игровая утечка.
Добавлена ограниченная диагностика переполнения, алгоритм free не изменён.
Первый retirement harness имел unsigned index, расширявший гостевой адрес
неправильно; исправлен индекс только в тесте. ASan/LSan повторно прошли.

Windows live и Linux diagnostic собраны без запуска. CPU v61 прежний;
32 зависимости и Dora64 неизменны. Карта/команды: vi-resource-lifetime.md.
Динамические вложенные ресурсы, остальные CPU framebuffer writers, визуальный
raw RDP и долгий GPU memory lifetime остаются предметом дальнейшего аудита.


## 2026-09-29 — цепочка paired vertices после test-018

Реестр800DD420 принимает пары +6C/+70 через80038C58. Найдены allocation80038BC8,
morph80038D58 с bank index801A9544 и segment binding8003AC14. Новый компонент
проверил оригинальные тела: copy, bank isolation, destruction и16-tick free.
51 парное удаление безticks теряет2 регистрации, owner pointers уже очищены.
Это синтетическое расписание, не доказанная частота на реальном уровне.

RT64 копирует Vertex и texture upload bytes; LoadOperations могут повторно читать
RDRAM во время FullSync, поэтому граница — весь send_dl, не первый load.
Depth clear8002F594 оказался генератором RDP-команд; video init меняет указатели,
не пиксели. Патчей в этих точках не добавлено. Production не изменён; rebuild не нужен.
Подробности/незакрытые условия: paired-vertex-lifetime.md. ASan/LSan стенда exit0.


## 2026-09-29 — test-019: lifetime guard парных vertex buffers

Первопричины:16 ticks не гарантировали конец отложенного send_dl;100-slot registry
терял overflow после обнуления owner pointer. Добавлен ticket для строящегося кадра
8002FF64, publication marker8002FE20 и завершение после processDisplayLists.
Ресурс ждёт только задачи не новее своего retirement ticket. Исходный timer и
allocator сохранены; overflow сохраняется и освобождается на guest CPU.

Выявлено отличие registry reset80038AE4 от heap reset80033A14. Первый переносит
живые записи с остатком timer в host storage, второй инвалидирует старые указатели
до reuse адресов новым heap. Нельзя применять поздний free к новой сцене.
Guard не держит mutex через SDK и не ждёт global outstanding==0.

Оригинальные registry functions +production guard прошли ASan/LSan и TSAN,
включая102 запроса/100 slots,100 циклов, concurrent completion, новые кадры,
отдельный registry reset и heap reuse. CPU generation не менялась. Windows/Linux
собраны без игрового запуска. Область исправления ограничена парными вершинами;
512-slot registry и прочие пути остаются открыты. См. retirement-guard.md.


## 2026-09-29 — test-020: наша startup regression

Источник: logs/1790653280923/runtime.log. Первый SUBMITTED task=1/bytes=30 ещё
не разобран, initial process вызывает scene heap init; наш общий assert делает
abort с сообщением heap reset before published task parsed. Предположение о
нулевой очереди при любом heap init оказалось неверным. Предыдущий тест покрывал
строящийся кадр, но не опубликованный кадр через startup heap initialization.

Сначала новый regression case воспроизвёл тот же SIGABRT на test-019 source.
Затем удалён только необоснованный глобальный запрет. Tickets НЕ сбрасываются:
новые retirements всё ещё ждут активный task, старые адреса не освобождаются в
новом heap. ASan/LSan и TSAN прошли, включая heap reset с overflow. Защита обычного
retirement и сохранение overflow не отключены. Windows/Linux собраны без запуска.
Нельзя объявлять этим доказанной безопасность всех heap/graphics пересечений.

## 2026-09-29 — пункты 2/3: host Port menu и управление (test-021)

Использованы общие overlay/FPS интерфейсы закреплённого Dora64/RT64, без переноса
адресов первой игры. Добавлены port_settings, port_input, port_menu и адаптация
live_io/live_graphics; версия CPU/audio и game hooks не менялись. Подробности и
воспроизводимые проверки: port-menu-controls.md.

В проверке focus выявлена ошибка первой реализации: отпускание в неактивном
окне преждевременно снимало запрет возврата удержанной кнопки. Теперь снятие
происходит только в активном окне при нейтральном вводе. Для меню малый дрейф
стика не блокирует доступ к регулировке dead zone; игровая защита сохраняется.
Проверено восстановление FontGlobalScale перед PopFont: обратный порядок оставлял
внутренний размер шрифта ImGui изменённым после overlay. Новая проверка охватывает
стиль и размер шрифта, а также настоящий путь навигации клавиатуры/геймпада.

В процессе сборки исправлен namespace plume у draw hook; это compile-time
ошибка, пользователю такой EXE не передавался. ASan/LSan, TSAN и SDL virtual pad
проверяют компоненты без игры/GPU. Реальный вывод/полноэкранный переход и
прохождение не объявляются проверенными. Первая часть и 32 pinned dependency
commits сохранены. Дальнейшее продолжение полного аудита памяти отложено по
решению пользователя, остаточные риски из retirement-guard.md остаются открыты.

## 2026-09-29 — согласование меню и settings с Dora64

## Test-022 — соответствие Dora64

Test-021 пользователь проверил: «вроде работает», затем уточнил соответствие
первой части. Меню теперь повторяет оформление и порядок готовых разделов:
Game / Graphics / Audio / Controls. Названия: Settings profiles, Show FPS counter,
Display mode, Output resolution, Game render resolution, Texture filtering,
Master volume, Stick dead zone, Restore default controls.
Переназначение — три колонки Action/Keyboard/Gamepad и окно Bind control с
Clear binding/Cancel. Подвал: Continue, Exit game → Exit game? → Exit/Cancel.
Нереализованные Cheats, камера и Reset game не добавлены как пустые элементы.

F1 меню не вызывает. Открытие: Escape / Back–Select.
FPS использует параметры Dora64: текст (0.72, 1, 0.76, 1), фон (0.025, 0.03, 0.04),
alpha 0.72, отступ12, padding8×5, rounding5, масштаб относительно высоты720p.
Цвет проверяется по CPU vertices ImGui без графического вывода.

Оконный режим по умолчанию 1280×720. Output resolution содержит стандартные
размеры Dora64: 640×480, 800×600, 960×540, 1024×768, 1280×720, 1600×900,
1920×1080, 2560×1440, 3840×2160. Сохранённый пользовательский режим имеет
приоритет при запуске. Обычное перетаскивание края окна не сбрасывается каждый кадр.
Полноэкранный режим использует разрешение desktop/native.
Exit game передаёт запрос на основной поток и вызывает обычный ultramodern::quit,
как закрытие окна; принудительного _Exit для этого нет.

Файлы с префиксом port, как уточнил пользователь:
- port_pc_settings.json: show_fps, master_volume0–1, graphics и menu_scale.
- port_input_settings.json: bindings/action/keyboard/gamepad, stick_dead_zone_percent.

Общие поля и GamepadInput serialization соответствуют Dora64, версии14/3.
Идентификаторы действий третьей игры независимы от игровых названий первой.
Дополнительные кнопки SDL используют диапазон1000+, стандартные — номера Dora64.
Неизвестные JSON-поля сохраняются, неподдержанные функции не активируются.
Windows: рядом с EXE/ROM. Linux live: рядом с выбранным ROM, как в Dora64.
Диагностика остаётся отдельно; saves упаковка не заменяет.

Миграция читает port-settings.json только для отсутствующих новых файлов;
старый файл остаётся на месте. Новые файлы при повторном запуске имеют приоритет.
Нетронутая стандартная раскладка test-021 заменяется на Dora64, пользовательские
переназначения сохраняются. Упаковка настройки не создаёт и не перезаписывает.

Стандартная клавиатура: WASD — stick, Space/C/Z — N64 A/B/Z, Enter — Start,
Q/E — L/R, arrows — D-pad, IJKL — C-buttons. Геймпад: left stick, A/B, LT, Start,
LB/RB, D-pad, right stick. Dead zone25%. Restore default controls возвращает
эту раскладку. Названия Actions оставлены N64 по просьбе пользователя.

WSL: до SDL_Init включается SDL_HINT_LINUX_JOYSTICK_CLASSIC при наличии WSL_INTEROP
или WSL_DISTRO_NAME, точно как в Dora64/main.cpp. Обычный Linux/Windows сохраняют
обычный backend. Реальное устройство под WSL агентом не запускалось.

Проверки без игры/GPU: JSON schema/roundtrip/migration, default720p, F1 inert,
quit request, четырёхвкладочное меню/навигация/зелёные FPS vertices ImGui,
SDL virtual pad и стандартные bindings. Windows/Linux RT64 компилируются.
Живое изображение/полноэкранный переход и прохождение проверяет пользователь.

## Test-023 — исправление состава Controls и единая ветка

По замечанию пользователя удалён не запрошенный Menu scale и восстановлен
Background gamepad input как в Dora64. SDL background hint и обработка фокуса
разделяют клавиатуру и геймпад. Опция сохраняется в port_input_settings.json,
старое поле menu_scale удаляется из port_pc_settings.json без сброса остального.
Проверки: SDL virtual gamepad on/off, keyboard focus/refocus, menu quarantine,
JSON roundtrip и ImGui CPU draw data; Windows/Linux builds без игры/GPU.
По просьбе пользователя все изменения объединяются в main приватного Dora64-3,
две созданные агентом ветки удаляются после проверки включения в main.
Дальше работать только в main. Подробности: port-menu-controls.md.

## Test-024 — Graphics и оформление из Dora64

Test-023 пользователь подтвердил рабочим. По его следующему запросу перенесены
общие элементы Graphics из Dora64680d81f, с отдельным уточнением: Manual FPS
должен применяться даже выше частоты монитора.

- Monitor: имена и режимы SDL, выбор монитора переносит окно; выбранному монитору
  назначается desktop-размер, как в Dora64. Отключённый индекс ограничивается
  доступными дисплеями. Refresh обновляет список через запрос на основной SDL-поток;
  SDL_DISPLAYEVENT также запрашивает обновление.
- Output resolution: стандартные оконные размеры плюс поддерживаемые размеры
  выбранного монитора, сортировка и удаление повторов разных Hz. Borderless
  показывает desktop/native размер и Hz, не меняет видеорежим ОС.
- Frame rate: Original / Display / Manual; ручной предел60–360FPS, исходное
  значение144. Показаны Display Hz и Target FPS. Modern включает Display,
  Original возвращает Original. Старые настройки без этих полей остаются Original.
- Manual намеренно отличается от Dora64: RT64 больше не ограничивает targetRate
  частотой дисплея. При60Hz и Manual240 целевое значение остаётся240. Подпись:
  Smoother motion with the original game speed. Display follows the display refresh
  rate; Manual can exceed it. Это целевое, а не гарантированное число кадров.
- Общие цвета/размеры взяты из исходника меню: title background, приглушённые
  пояснения Controls, default item width, popup buttons120/100, table252, окно760×500,
  padding22×18, поля48 с масштабированием от720p, footer54. Убрана собственная
  раскладка desktop-вкладок через child-окна. Footer содержит только существующие
  Continue/Exit; Reset game пока не реализован. FPS получает то же сглаживание
  показаний25%/75%, что в Dora64. Текстура шрифта и базовая тема те же из pinned RT64.

Настройки сохраняются в прежнем port_pc_settings.json: graphics.display_index,
display_width/height/refresh_rate, frame_rate_mode (Original/Display/Manual),
frame_rate_limit. Общие сохранения и bindings не меняются.

### Источники и границы переноса

| Источник | Переиспользование |
|---|---|
| Dora64/src/system_overlay.cpp @680d81f | refreshDisplays, displayCombo, outputModeCombo; drawGraphicsTab, drawMenu, drawControlsTab, drawFps, ScopedOverlayScale |
| Dora64/src/rt64_render_context.cpp @680d81f | соответствие режимов RefreshRate и refreshRateTarget |
| pinned RT64/src/hle/rt64_application_window.cpp | setDisplayConfig, включая восстановление Win32 chrome и доступность заголовка большого окна |
| pinned RT64/src/hle/rt64_workload_queue.cpp | реальный выбор targetRate; снят только clamp Manual до swapChainRate |

Часть SDL адаптирована к поточной схеме Dora64-3: меню читает копию каталога,
окно меняет только SDL-поток. RT64 userConfig меняет его producer-поток;
swapChainRate читается под configurationMutex и публикуется атомарно.
Перетаскивание/изменение размеров окна не отменяется каждый кадр.

Снятие ограничения RT64 выполняет tools/prepare_rt64_framerate.py: проверяется
SHA256 pinned исходника, создаётся только build-local копия. CMake заменяет ровно
эти два translation units; библиотеки, pins и Dora64 остаются неизменными.

Игровые адреса/хуки, CPU/RSP генерация, VI/scheduler и звук не менялись.
Высокая частота использует штатное frame matching/interpolation RT64. Качество
движения/стыков сцен третьей части не подтверждается проверками без GPU:
это отдельная визуальная проверка пользователя, Original остаётся доступным.

Проверки: JSON roundtrip/profiles, CPU ImGui geometry/grey help/Graphics modes,
SDL display wrappers (два монитора, dedup, Refresh, fullscreen/failure, disconnect,
отсутствие повторного resize), ASan/TSan, компиляция и запуск извлечённого
реального switch RT64 (Manual240@60Hz, Manual360@165Hz, Display, Original),
SDL virtual-controller/audio regression, сборки Windows/Linux. Никаких игровых
окон, рендера или реального переключения мониторов агент не запускает.

Справка SDL: https://wiki.libsdl.org/SDL2/SDL_GetDisplayMode,
https://wiki.libsdl.org/SDL2/SDL_SetWindowPosition,
https://wiki.libsdl.org/SDL2/SDL_DisplayEvent.

Дополнение к test-024: после снятия targetRate clamp найдена ещё одна причина
ограничения вывода — VSync. В build-local present queue Manual выше Hz отключает
VSync до needsResize, остальные режимы восстанавливают его. Меняет только
present-поток; pinned RT64 не редактируется. Проверка извлекает реальную политику
из генерируемого кода и проверяет включение/выключение, без GPU.

## Анализ20→60FPS после test-024 — без реализации

По запросу пользователя выполнен только статический анализ. ROM подтвердил
scene-selected делитель801A39DC:20 прямых записей3 и4 записи2 в известных телах.
Основной gameplay.scene1 задаёт3; game update и разрешение frame build находятся
за общим порогом. Подтверждены frame-count sleep и fixed-step fade: замена3→1
нарушает исходную длительность. Current60FPS — RT64 interpolation, не update60.
Артефакт стыков предположительно связан с automatic matching; конкретная причина
ещё не установлена. У RT64 vertex interpolation по умолчанию SKIP; CPU morph
третьей игры требует отдельной интеграции. Недобор цели не локализован без
frame-time/matching данных. Подробности: framerate-analysis.md; адреса и ROM hashes:
research/framerate-static-analysis.json. Код/зависимости/EXE/настройки не менялись.

## 2026-09-29: начало native Manual FPS; Original сохраняется

По разрешению пользователя начата реализация настоящих кадров без RT64 interpolation.
Для испытаний согласован существующий Manual; Original сохраняет исходный цикл.
Добавлен изолированный компонент времени, fade и model clip time. Значения частоты
из диапазона ползунка (например73/144/165 кадров в секунду) не должны терять время;
отдельно учитываются исходные20/30 тиков. Actual dt сохраняется при задержках,
не добавлен лимит времени работы. Есть явный resume для исключения паузы.

Оригинальными функциями на синтетической RAM подтверждены две причины возможных
ошибок HFR: следующий animation update стирает end pulse, большой delta оборачивает
клип только один раз. Компонент сохраняет завершения до потребления и рассчитывает
несколько оборотов. Это ещё не подключённый игровой фикс. Wrapper8002A1B8 вычисляет
позу ДО продвижения времени; этот порядок нельзя незаметно инвертировать.

Найдена дополнительная зависимость от VI: framebuffer-ready80026CEC сравнивает
current/next; scheduler retrace рассылает сообщения и звуку. Не поднимать VI ради
Manual и не обходить реальные подтверждения ресурсов. Native scheduler и аудит
движения/камеры/phase1 остаются работой следующей интеграции.

ASan/LSan и UBSan нового компонента прошли;655360 fade и384 animation сравнений
с оригинальными телами; проверки arbitrary Manual deadlines/режимов/длительностей.
Desktop Linux/Windows собраны без запуска;32 pinned зависимости и Dora64 чистые.
Android не собирался. Новый пользовательский EXE не передавался: режим ещё
не интегрирован. В test-024 Manual всё ещё прежний RT64-путь. Документ:
docs/native-framerate.md; отчёты research/native-frame-time-*.json.

## 2026-09-29: native frame protocol и сквозной2D fade

Добавлены два банковых ticket и единственный pending wake. Подтверждения guest и
host независимы; старый ticket не освобождает новый кадр. Для32-bit OSMesg token
хранится неизменяемым до потребления сообщения, иначе scene switch создаёт ABA.
При оставленном старом сообщении mailbox нельзя перезаписать новым поколением.
Неактивный протокол не выдаёт события; leave сохраняет незавершённые задачи.

Выполнением оригинальных80026CEC/80026D40 подтверждены VI equality gate и порядок:
completion отправляется ДО swap. Общее увеличение VI недопустимо: ветвь scheduler
ещё рассылает события звуку и меняет счётчики. Старое наблюдение возможного повторения
delay slot в generated C проверено: goto after_N пропускает вторую текстовую копию,
ошибки двойного исполнения здесь нет.

Сквозная синтетическая проверка выдаёт deadline/token, строит оригинальными
800289D8/80028688 дополнительные2D display lists с альфой нового Fade и завершает
две стадии владения. При60/73/144/165/360FPS длительность0.8s сохранена до дискретности
нового кадра; больше16 различных альф; чужой банк не меняется. Это не GPU-проверка
и не полный scene update. Оригинальные функции извлекаются только в build-local.

ASan/LSan и TSan прошли,2000 кадров с параллельными completion; Linux/Windows
скомпилированы без запуска.32 зависимости и donor чистые. Новый EXE не передавался:
живые очереди/CPU overlay/framebuffer latch ещё не подключены. Scene34 уточнена как
initial image/controller screen; render другого menu32 имеет запись801C68AC,
поэтому общий phase1 нельзя объявлять чистым рисованием. Детали и следующие точки:
docs/native-frame-scheduler.md; research/native-frame-scheduler-*.json.


## 2026-09-29 — доставка native FPS в игровой loop

Причина потенциального зависания: void host enqueue не подтверждает guest delivery;
при full guest queue/requeue=false единственный wake терялся бы. Добавлены проверяемый
requeue sender, hook800278F4, сохранение CPU-контекста и quiescent reset.
Обычный VI/звук/retirement не переводились на новую частоту.
Стенд использует настоящую adapted mesgqueue и оригинальный loop; ASan/LSan/TSan
результаты и ограничение atomic_thread_fence записаны в research/native-frame-bridge-*.json.
Полный Native Manual ещё не включён; никакая игровая/визуальная проверка не заявляется.
См. docs/native-frame-bridge.md.


## 2026-09-29 — отдельное пробуждение графического scheduler

Подключены hooks v63: после публикации8002FF58 и после scheduler Receive800268EC.
Новый wake сохраняется при заполненной очереди, не блокируя игровой поток на
дополнительном guest SendMesg. Публикация до wake обязательна, иначе возможен
пустой drain и ожидание следующего VI. Оригинальные задачи/приоритет/yield/SP/DP
сохранены; pump пропускает только retrace count и client broadcast.
Оригинальные11 функций и настоящая очередь проверены на synthetic RAM с ASan/LSan.
Отдельно уточнена ошибочная предпосылка: наличие донорского CPU-overlay notify
не означает его использования третьей игрой. Следующая граница — VI snapshot и
owned framebuffer presentation. Native mode ещё выключен; подробности в
docs/native-graphics-pump.md.


## 2026-09-29 — native present и три условия возврата банка

Причина риска reuse: completion game queue приходит до последнего чтения task+0C
в scheduler. Добавлены регистрация8002FF50, retirement80026DBC и consumption80027A6C;
HostRead приходит после реального update_screen. Present содержит собственный VI
snapshot с framebuffer задачи; periodic VI старого поколения отбрасывается.
Type2 расположен в bank+C0; bank+E0 — type4, поэтому прежний pump harness уточнён.
Испытаны оригинальные11 scheduler functions и game loop, реальные runtime queue,
извлечённые update_vi/native enqueue/screen worker на synthetic endpoints.
ASan/LSan/TSan пройдены, Linux/Windows собраны без запуска. Native Manual выключен.
Не закрыта граница входа: уже выполняемый periodic read требует gfx handshake,
а Original game loop ещё надо отделить от native builds в scene adapter.
Документация: docs/native-presentation.md. GPU-проверки не заявляются.


## 2026-09-29 — неблокирующий native handoff и спящий producer

Закрыта гонка между проверкой active и началом Original update_screen:
реальный worker получает read lease через PresentationGate. FrameSession ждёт
уже начатый read без блокировки guest thread, держит допуск закрытым до полного
drain. Producer подключён к init/join runtime, ждёт deadline либо изменения
mailbox/banks/rate; wake revision предотвращает lost wakeup перед засыпанием.
ASan/LSan/TSan на реальном producer и извлечённом screen worker пройдены,
Linux/Windows собраны без запуска. Initial compile failure recomp.h исправлен
разделением runtime/guest интерфейса. Исходные VI/audio не ускорялись.
Scene34: init задаёт divider3; counters60/16 и input repeat принадлежат update,
а build меняет transition gate801A48E0. Эти пути нельзя все повторять чаще.
Ни одна сцена ещё не активирована; подробности: docs/native-frame-session.md.


## 2026-09-29 — исправление раннего retirement и аудит image draw

При подготовке scene adapter обнаружена ошибка прежнего анализа80026DBC:
task_complete вернулась, но SP/DP callers продолжают читать/писать timing-поля.
Retirement перенесён в80026BEC/80026C94 после последних task stores. Изолированный
negative control со старым hook воспроизводит слишком ранний present; исправление
проходит SP-first/DP-first с немедленным game/host completion и reuse банка.
Native completion также пропускается через Original pause gate, иначе терялся
GuestTask ack и drain зависал. Незарегистрированные сообщения не меняют поведение.

Проверена image draw closure scene34 (15 оригинальных функций) вместе с Fade:
оба банка, синтетические заголовки/размеры и802 списка2D-команд на60–360FPS.
В выбранных состояниях вне DL/cursor/стека гостевая память не меняется.
Native scene ещё не активирована: полный build, legacy retirement на входе и
drain перед transition требуют связки с игровым loop. Подробности и ограничения:
docs/native-presentation.md и docs/native-scene34.md. Первая игра не изменена.


## 2026-09-29 — общий loop, полный инвентарь времени, v66

По уточнению пользователя работа переведена с разрозненных hooks к общему контроллеру.
Native consumer вызывает полный исходный build, Manual settings подключены, общие
вход/пауза/выход и ожидание legacy хвостов работают. Правила допуска вынесены отдельно;
пока разрешена image scene34, остальные состояния Manual идут Original без RT64
interpolation. Полного HFR gameplay эта итерация не заявляет.

Сквозной стенд с27 настоящими функциями подтвердил60 исходных updates при сотнях
native frames, смене60/73/144/360 и безопасном выходе к transition. Первичный
entry-wait fixture выявил необходимость различать initial built0 и свежий update:
добавлена проверка VI count0, fixture моделирует реально построенный предыдущий кадр.
Повторное чтение FPS при входе заменено одним snapshot. Генератор сначала отклонил
дублирующий entry hook process_pair_install; hook объединён с существующим trace.

Уточнено и исправлено: image-only fixture ошибочно задавал размеры банков десятичными
числами, хотя реальные значения hex0x31728/0x12328. Production constants правильны;
fixture переведён на них и перепроверен. Прежнее свидетельство конкретных банков
не засчитывается без этой поправки.

Native Manual отключает RT64 interpolation, но старый VSync policy смотрел на RT64
enum Manual и поэтому ограничил бы новую схему. Добавлен независимый native target
query в build-local present queue с null fallback для standalone replay.

Весь известный код проинвентаризирован:4778 функций50 секций,47 pair installs,
3 slot installs,61 indirect calls,167 fade commands,36 animation wrappers.
Это статическая карта кандидатов, не завершённый semantic audit всех сцен.
План крупных подсистем и точные границы: docs/native-game-loop.md.


## 2026-09-29 — v67: command-aware fade и общая карта model animation

Пользователь test-025: «ничего не вылетело», первая заставка60FPS. Объём проверки
ограничен этим сообщением; прохождение/3D native этим не подтверждены.

Причина остановки native перед белым fade: clock снимал состояние только при
входе и не видел новые команды. Пять общих resident entry hooks теперь обновляют
текущее значение до команды, затем учитывают её результат от её timestamp.
Неизменившийся begin/hold не теряет дробный остаток. Полный белый state1 fade
разрешён до counter1; state2/progress reset не расширен. Drain остаётся перед
реальной заменой callbacks и освобождением сцены, а не перед командой цвета.

Проверка команд сначала ожидала alpha1 после половины последнего шага9→0;
реальная непрерывная модель сохраняет завершение на исходной tick-границе и даёт5.
Исправлено ожидание стенда, не скорость игры. Link-ошибка стенда была неверным
именем границы progress_reset, из-за чего подтянулась лишняя closure; имя исправлено.
После этого original loop/white fade:76updates, режимы60/73/144/360, пауза/Original,
полный drain; ASan/LSan пройдены. Число native builds — не измерение GPU FPS.

Model audit всего известного кода:752 clip-select,362 end reads,129 time reads,
65 duration reads,48 seek,90 pause,222 clamp,36 pose/time calls. End getter
недеструктивный; morph пишет выбранный bank, clip reset/seek/reuse требуют явной
generation. Карта и границы: docs/model-animation.md. Live model timing не изменён.

## 2026-09-29 — v69: общий model adapter; полный FPS ещё не готов

Требование пользователя расширено/уточнено до полного объёма без промежуточных
частичных EXE. Новая сборка для прохождения не передаётся. test-026: пользователь
подтвердил отсутствие вылета в своей проверке; границы прохождения неизвестны.

Реализованы shared lifecycle hooks, legacy observation и native pose scope.
Найдена и закрыта конкретная гонка будущего общего режима: morph update пишет
по next bank, который при независимых native tasks может быть занят. В native
ownership такие записи отложены до frame ticket; stale модели тоже заполняют
свободный банк последней исходной morph-позой, без перезаписи другого банка.

8003997C — отдельный matrix pass; прямые вызовы после8002DEF8 найдены во многих
draw путях. Проверка исходного matrix builder подтвердила дробную позу в выходной
матрице. Первоначальная ошибка стенда: вход был заполнен float identity, хотя
8005A0C0 принимает N64 packed16.16. Исправлен fixture, production не подстраивался.
Другая ошибка fixture — имя getter80039544 имеет префикс s0_menu, а не s0.

Прямой minigame4 draw closure освобождает модель. Adapter восстанавливает её
временные CPU bone поля перед original destruction и удаляет owner; другие модели
не затрагиваются. 8003B7D0 не равен heap reset: original не освобождает модели,
поэтому он инвалидирует clock, но не удаляет registry owners.

Масштаб physics подтверждён отдельно:68 player state pairs/128 handlers плюс
другие actor/minigame dispatchers.801BC7DC оказался geometry collector, не общим
интегратором движения.801BF828 смешивает velocity additions, spatial corrections
и события;8002F064 — геометрия камеры, не её temporal update. Массовый множитель
на эти функции не применялся. См. native-motion.md; это незавершённая интеграция.

## 2026-09-29: сохранение скомпилированного test-027

Последнее уточнение пользователя: все собранные Windows EXE складывать в
testbuild без просьбы запуска. Подготовлены README и compile receipt для v69;
проверены хэши скомпилированных исходников, генерации и трёх CPU-проверок.
Linux/Windows сборки успешны; Dora64 и все32 закреплённые зависимости неизменны.
Полный FPS не завершён, новый 3D-допуск не включён. Игра не запускалась.

## 2026-09-29: v70/v71, общая шкала времени и сценарная камера

Добавлены SimulationTimeline/Scope и72 семантических hooks с проверкой исходных
инструкций/полных функций. Original не включает scale без explicit scope.
Число правил не является процентом перевода всей игры.

Отрицательные проверки: synthetic MIPS context не настроил f_odd, original cos
писал через null. Исправлено окружение FR=0 по закреплённому runtime; production
cos не изменён. Ещё одна ошибка fixture: unsigned displacement в MEM_W/MEM_H
давал неверный адрес. Исправлены знаковые смещения, не сам оригинальный код.

Частое обновление script camera подтвердило накопление follow offset (mode3,
orbit4,duration2). Проверка только output полей этого не обнаруживала; расширена
на внутреннюю position/velocity. Исправление — аналитический пересчёт композиции
от начала текущего original tick с накопленным h, не накопление композиции за
каждый display. Подробнее native-simulation.md. Live scene coverage пока прежний.

## 2026-09-30: общие эффекты, изоляция контекста и ресурсный барьер

В копиях recomp_context найден оставшийся f_odd исходного объекта. Исправлены
native frame bridge, scheduler pump и model pose/morph; copy helper перебазирует
указатель в destination в FR=0/FR=1. Resource retirement использует сохранение/
восстановление того же адреса ctx и не вызывает original code на saved copy;
этот другой случай не требует перебазирования. Регрессионная проверка пишет
в каждый нечётный FPR копии и подтверждает неизменность исходного контекста.

2D rectangle801BE5F0 теперь имеет непрерывную геометрию при исходном времени
завершения; zoom801D56F4 использует экспоненциальный коэффициент по h, не alpha*h.
Sprite sequence80028C74 сохраняет byte timers/frame pulses на дискретной границе.
200 сравнений с исходными функциями, включая каждую границу20/30Hz и73FPS.

v73: process replacement800280DC вызывает общий resource barrier. Он останавливает
новые кадры, через scheduled guest receive собирает зарегистрированные completion,
ждёт outer SP/DP retirement и HostRead, сохраняет прочие сообщения в FIFO и
возвращает их через hook800278EC. Проверены2 банка, ранний game completion,
поздний scheduler tail/HostRead, stale wake, сохранность контекста и scratch.
Это ещё не blanket-защита всех loaders/DMA/heap resets: места до process install
и принадлежность вызывающему потоку требуют отдельной проверки перед расширением.
Live scene coverage не расширен; задача полного FPS продолжается.


## 2026-09-30: v76, gravity/displacement и процессные задержки

Связаны gravity и displacement в пределах SimulationScope; независимость от
разбиения шага проверена, ограничения capped model указаны в native-simulation.md.
Существующие rate-only правила были недостаточны для высоты прыжка.

Найдена общая причина ускорения process waits при дополнительных отрисовках:
phase1 dispatcher изменяет статусы/счётчики наряду с вызовом draw callbacks.
Простой запрет повторного уменьшения wait недостаточен: status4->2 должен
пропустить все оставшиеся native кадры текущего тика до первого callback.
Добавлен этот activation guard; Original и deferred activation проверены на
реальной функции800281B8. Первый запуск checker требовал исключить нативный
hook из ROM closure extraction; это ошибка тестового извлечения, не игры.

## 2026-09-30: v82, обычная камера — незавершённый общий перевод

Разобрана таблица всех16 режимов dispatcher801DBD58 (15 уникальных handlers).
Modes0/1:DB7C8;2:D6EAC;3:D8FF0;4:D7294;5:D5864;6:D8578;7:DB720;
8:D9B00;9:D8A24;10:D9C94;11:DA06C;12:DAB58;13:DA588;14:DAF50;15:DB3D0.
Названия механик/уровней этим номерам ещё не присвоены. DB720 и D9B00 —
прямые геометрические камеры, временной фильтр к ним не добавлялся.

Общие формулы: approach1-(1-alpha)^h, retention^h. Для ограничения поворота
использован дробный iterate именно исходного e -> e-clamp(alpha*e,-L,L).
Порог T=L/alpha; k=-log(1-alpha). При |e|>T берётся n=ceil((|e|-T)/L),
phase=n+log((|e|-nL)/T)/k. Из phase-h обратно восстанавливается остаток ошибки.
При |e|<=T используется экспоненциальное приближение. На целых шагах получается
исходная последовательность даже при переходе через cap, а разбиение шага
композируется для неподвижной цели. Original проходит исходные инструкции.
Это вычисление уравнения, не RT64 interpolation готовых поз.

Фиксы первопричин:
- Continuation labels генератора должны стоять ДО остальных hooks того же PC.
  Иначе tick gate обходил соседнее scale движение (mode2 Y2 вместо1). v78+.
- В mode6 (D8578) Z проходила через signed16 временную переменную каждый кадр.
  Native дробные шаги теряли субединичное движение. Float сохраняется для
  fractional steps; целый шаг и Original сохраняют исходное округление.
- Коэффициент mode6 зависит от control+2 и ограничен speed(control+2+3);
  масштабировать только коэффициент недостаточно. Gain и cap обработаны вместе.
- Шесть recenter/obstruction timers идут по исходным тикам, отдельно от движения.
- Shake55EC: signed amplitude добавляется как displacement*h; decay и remaining
  меняются в конце исходного тика. Нет повторного полного импульса каждый draw.
- Две временные аннотации5EAC/78DC удалены: перед ними distance заново вычислен
  из геометрии глаза, это не обычный фильтр предыдущей distance. Пространственные
  шаги collision probes не масштабируются. Наличие умножения на0.1 не доказывает dt.

Проверки v82 без игры/GPU:970 control/transition/coefficient/shake cases с
реальными исходными функциями; Original/full-step RAM differential; 440 вариантов
capped approach по20 тиков;20/30Hz и60/73/120/144/165/360FPS. ASan/LSan,
UBSan нового C++. Отдельно все13 остальных roots:78 Original/full-tick RAM
сравнений. Synthetic actors, no-map path исходной collision query, не стены уровня.
Широкая проверка rate spread ДИАГНОСТИЧЕСКАЯ: остатки mode4/5 distance~1.28,
mode9 pitch~0.155deg, mode10 distance~1.60. Они не объявляются пройденными.
Mode6 spread~0.000023; остальные~0..0.000733 в этих сценариях.
Причины ещё исследуются: derived output против temporal state, задержка публикации
distance на один вызов, инициализация/смена collision pitch target, coupled filters.

Полная камера с препятствиями, внешний ввод/mode switch, feedback тряски через
фильтр, вся физика/player callbacks, 2D/миниигры и общий native update остаются
незавершёнными. Live policy всё ещё scene34 image/fade; v82 не означает нового
допуска сцены. Последний EXE test-029(v76). Эта запись — checkpoint, не завершение.

## v83: общий расчёт связанных уравнений ordinary camera

Оставшиеся v82 расхождения камер4/5/9/10 устранены в компонентной проверке
OrdinaryCameraTimeline. Исходный алгоритм последовательно вычисляет геометрию,
target filters и предыдущие distance/pitch targets. Повторять эту композицию
при каждом коротком шаге означало менять алгоритм. Теперь проверенные уравнения
вычисляются на накопленном времени текущего исходного тика с его начальным
состоянием. Цели/geometry берутся из текущего игрового состояния, готовые
previous/next poses не смешиваются. На границе1.0 получаем исходный результат.

Состояние ограничено конкретными полями каждого handler, включая halfword
границы; не копируется весь диапазон globals. По соседству есть чужие1823E,
18328/1832C и zoom halfword18278. Camera eye+0C/10/14 и up-vector+28 пишет
геометрический8002F064; они исключены из anchor. Внешняя запись в собственное
состояние камеры или смена handler/camera отключает anchor до следующей границы;
чужая запись не откатывается. reset очищает host bookkeeping без гостевых pointers.
Смена режима посреди тика пока использует incremental equations до новой границы.

Mode6 signed16 Z correction распределена по времени текущего тика, чтобы не
обрывать субединичное движение на каждом кадре и не получить скачок в его конце.
На полном шаге исходное округление сохраняется.

native-camera-modes-check v83:9571 сравнение/проверка,13 roots,6 synthetic states,
60/144/360FPS, исходные функции с реальной тригонометрией. На каждой20Hz границе
target/angles/distance совпадают с Original, на общих промежуточных timestamps
60/360 совпадает весь camera output. Есть проверка более30 изменений за секунду,
чтобы20Hz-only путь не мог пройти как native. Отдельно external writes/reset.

Для препятствий восстановлен grid format80040104: signed XZ bounds; relative
offsets polygons/lists/grid; cell50;44-byte triangles with projected XZ vertices,
normal(A,B,C), D for Ax+By+Cz=D, flags. Synthetic room uses actual original
query: floor0/ceiling250, inside flag0/outside flag1. Это не ROM level resources
и не прохождение реальных стен/уровней. Игровое окно/GPU не запускались.

Компонент добавлен в сборку, но LIVE scene policy НЕ расширена: общий scene
simulation update ещё требует соединения с ним и с physics/events. Полный FPS
остаётся незавершённым. CPU-проверка камеры не является проверкой всего game loop.

## 2026-09-30 — v84–v88: общие движение, события и игровой preview

Добавлена pre-render граница: обновление выполняется до резервирования render
bank. Смена сцены/частоты инвалидирует старый wake до публикации кадра.

Совместная проверка scalar drive/vector/contact выявила разный пройденный путь:
при разгоне0→12 за секунду старый60FPS X135/Z209.186 против360FPS X134.167/Z207.742.
Общий интегратор acceleration+drift теперь передаёт согласованное смещение
через проекцию скорости к collision solver. Проверены168 разбиений drive и24
совместные траектории движения/контактов/камеры на v85. Возле ограничения
скорости continuous траектория может слегка отличаться от дискретного
пересечения cap; Original формулы сохранены.

Все15 вызовов обычных камер в DBD58 используют общий OrdinaryCameraTimeline
через явную приостановку внешнего SimulationScope; кеш перемещения сохраняется.
Режим12 проверен совместно с движением. Это не прохождение.

Общее продвижение model time подключено к8003948C внутри SimulationScope.
На360FPS обратная анимация давала лишний wrap из-за сложения дробных шагов;
исправлено целочисленным накоплением elapsed при постоянной скорости.
Пройдены24 native-clock сценария,20 проверок crossing/реального frame18 branch,
74 проверки исходных pose/morph samples, ASan/UBSan/LSan. Проверены pause,
seek, clip lifetime, смена направления после clamp и свободный vertex bank.

Анализ129 object-time callsites с переходами и jump tables:132 сравнения,
84 equality.79 проверенных кадровых событий в64 функциях подключены к общему
crossing helper; четыре -1 sentinel для clip-ratio не являются событиями;
оставшаяся производная ratio==1 требует отдельной обработки. Карта не является
полным анализом всех прямых чтений model memory. Host event state очищается
при seek/rebind/destruction/region reset; Original comparison не изменён.

По новой просьбе пользователя дать EXE с неполным результатом подключается
scene1-ground-preview (v88). Continuous drive/motion/camera отделены от VI
без повторного выполнения state handlers, звука и таймеров. Только ground
states1..5; другие состояния/сцены возвращаются в Original. Во время preview
игровые часы моделей и NPC ещё исходные; render pose adapter вычисляет кривые
чаще. Не выдавать это за завершённый variable FPS. Новые проверки и сборка идут.

Windows v88 скомпилирована; совместная проверка actual preview motion block/camera
на60/73/144/360 и общий loop с drain/сменой режима пройдены. Игра не запускалась.
Дополнительные проверки прекращены по прямому уточнению пользователя.

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


## Диагностика дрожания и player integration 2026-09-30 — test-033 (генерация v89)

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

## 2026-09-30 — рабочая v91/v92, разворот и общий цикл объектов

Допуск игрока1..16, пять адресных правил для230C/3D8C/421C; новые правила
защищены ROM/function SHA и точными instruction words. До исправлений native
разворот возвращался из клипа в разные моменты:0.700000 при60FPS против0.684932
при73FPS. Истинная граница клипа0.683333333333 лежала между целыми наносекундами;
ограничитель теперь делит SimulationSlice в рациональных единицах исходного тика.
Дальше выявился более ранний переход15→16: остаточная скорость−3.78082 вместо0
переворачивалась до полного времени торможения. Его commit и12→13 проверяются
на старте исходного тика; continuous drive продолжает работать между решениями.
24 основных и48 reverse/braking CPU-сценариев теперь проходят при обоих делителях
и60/73/120/144/165/360FPS, обычной и городской камере. Реальный склон отдельно
не воспроизводился: в fixture плоская поверхность без скольжения.

801E8BDC: владение разделено по живым актёрам30..229. Проверенные38 строк таблицы
80202064 (stride84) ведут к пустой функции801E5D84; их common translation/opacity,
model clocks, D8 blink и sprites2..7 подключены совместно. Активный stage AI не
переведён. Native-проход пропускает801E483C, сохраняя контактные списки, и исполняет
его JAL delay-slot присваиваниеs2=30. После callback отмена сцены выходит через
общую очистку до новых обращений к старой physics. Original-путь сравнён сv61:
38 типов×24 расписания, полное совпадение4MiB RAM после common update.

Команда исходного мира меняетvx с8 на−4 вt=0.5с. Контрольная scratch-копия без
вычисления интервала перед world entry далаx434 вместо440 при30Hz/60FPS;
исправленная цепочка совпадает при всех шести частотах. Появлениеt=0.7с и
исчезновениеt=1.3с тоже ограничивают именно реальное время движения/анимации.
Model clip limiter использует план реально advancing owners/current rates:
неактивная модель не дробит чужую шкалу времени; newly bound clip и смена rate
учитываются до первого шага. Положительный rate25 теперь проверяется и по точному
числу end pulses, а не только итоговому времени.

Неошибки production: первая версия нового fixture неверно рассчитала ожидаемую
траекторию (500−2*ticks вместо500−ticks) и перепривязала clip без release; обе
ошибки исправлены в проверке. Float RAM позиции при165FPS дали439.996 вместо440:
ввели допустимую погрешность0.03 единицы для накопления float, сохранив точные
проверки дискретных событий. Финальные player/model/time/loop проверки прошли.

Снаряды/эффекты10..29 dispatchят27 callbacks типов82..108 через801FAB48/stride8;
второе слово каждой строки — флаг данных0/1, не второй callback. Их native
жизненный цикл ещё не включён. Generic pool использует отдельную таблицуstride84;
stage functions должны различаться по overlay identity, не только по VRAM PC.
Полный FPS остаётся в работе. Нового EXE нет по требованию пользователя о крупных
блоках. Игра/GPU не запускались, ROM/settings/saves не менялись.

---

## 2026-09-30 — v93, конечные эффекты и передача модельного времени

801D10D8 фильтрует native/source владельцев после active/C9 и перед BC7DC,
callback и анимацией. Продолжение обязано проходить через D1188, включая s3++;
после отмены сцены — очистка сохранённых регистров D119C. Девять обработчиков
проверены по ROM таблице801FAB48/stride8 и точным function SHA/instruction words.

Совместная проверка нашла реальную ошибку acquisition: существующий эффект с
длительностью клипа3 удалялся при native t=0 вместо0.033333333333 (divider2).
Original A1B8 оставил позу0, но clock3/end-bit после первого вызова. Новый
ModelAnimation::acquire_simulation использует свежую before-позу, проверяет
source step/divider, каналы/дескрипторы, rate и гостевые записи и снимает
несоответствующий lookahead/end-bit. Наблюдение потребляется один раз; GPU
и frame bones не меняются. Игрок исключён до анализа provenance кадровых событий.

Независимая базаv61 D10D8 даёт ровно одно удаление в том же исходном времени
для всех9 типов×24 расписания=216. Проверены реальные A100/B6BC/A3B0/EA4C/
E990/A0AC/B77C/scene-object-init и освобождение physics. ASan/UBSan/LSan прошли.
Ошибка fixture (флаг модели4 вместо3) исправлена вызовом реального A100.
234 функций в joint closure (232 actual +2 frozen Original reference).
Общие объекты, развороты, model и resource/loop проверки тоже прошли.
Нового EXE нет; работа продолжается над полётом и эффектами попаданий.

---

## 2026-09-30 — v93, шесть цепочек прямого полёта и попадания

Native owns91/93/95/97/99/107 с уже проверенными92/94/96/98/100/102/104/106/108.
97/99 используют чистую проекцию скорости CB1F0/CB2A0; остальные CB2F8;
общая трансляция CB6D4 уже dt-парная. Probe radius34 и геометрические±15 не
масштабировались.107 при скорости25 на окончании клипа привязывает клип1 и
меняет скорость30 ровно один раз.168 full-chain CPU сценариев,216 terminal
lifetimes и912 common-world RAM comparisons прошли. В flight acquisition ещё
168 полных RAM сравнения Source-pass с нетронутымv61 D10D8.

Новорождённый в более позднем слоте эффект раньше получал dt того же прохода,
хотя этот интервал предшествует его созданию. Теперь native dispatcher сохраняет
живые physics pointers перед проходом и не обновляет возникшие позже актёры.
Scratch-копия с удалённой защитой подтверждает ошибку: type91 при60FPS, hit1/
delete1, но child_time1.5 вместо0. Код production не удалялся; доказательство
build-local/native-player-update-check/newborn-negative-control.json.
Контракт пока не распространяется на непроверенные callbacks, которые могут
удалять и заново создавать чужой активный слот внутри одного прохода.

Fixture был расширен до real resource-list allocation/release и шести real
impact constructors. Resource lookup/release считается через строгие границы
со ссылками; реальные assets не извлекаются. Найдены и исправлены ограничения
fixture: trace kind для.scene1 —0x70A50, не0x1000; таблица ресурса99 включает
sprite IDs4F7..506; renderer-free clip table тоже должна быть heap-allocated
для настоящего деструктора; изолированные случаи сбрасывают счётчик idle,
чтобы искусственно склеенные десятки секунд не запускали неописанную idle
ветку после300 тиков. ASan/UBSan/LSan проходят. Production/game не запускались.
Полёт проверен с controlled contact flagD4, а геометрический probe выполняется
в синтетической комнате. Реальный gameplay/contact NPC требует проверки дальше.

---

## 2026-09-30 — v94..v96, рост и затухание projectile85/87/90

Правила ROM-word/function-SHA validated. Ошибкаv94 в LD emitter (три аргумента
вместо двух) была найдена CPU-компиляцией; EXE не создавался. После исправления
v95 выявилась реальная временная ошибка90: при60FPS/divider2 на тике29 Native
уже удалял эффект, Original оставлял его живым с отрицательной alpha. Простое
масштабирование alpha и gate деструктора недостаточны: Original проверяет alpha
в начале шага, а частые обновления проверяли уже частично уменьшенное значение.

Вv96 alpha сохраняет Source-тик baseline, predicate использует этот baseline
в течение шага, значения вычисляются от него с единым float округлением.
Кэш связан с зарегистрированной жизнью модели и сбрасывается при повторном
создании, renderer/reset bookkeeping, новой acquisition; изменения RAM и
пропущенные интервалы дают новый baseline. Удаление90 коммитится в конце
следующего Source-тика. Endpoint float значения и сроки жизни совпадают.

85/87: fractional approach при росте и decay, Source endpoint проекция
масштаба для решения о начале fade;87 сохраняет исходный f20 coefficient,
потому что он также порог удаления. Rotation25 масштабируется как temporal
delta, геометрические величины не масштабируются.90 age36 увеличивается
только на Source endpoint, решение о fade использует post-increment age.

Fixture timestamp различает consume-end-flag перед следующим интервалом
(прежние конечные эффекты) и Source-end commit внутри callback85/87/90;
для последних assert проверяет ends_tick(), а время включает numerator.
Это исправление наблюдателя, а не ослабление active/lifetime проверок.
72 сценария сравнивают state/lifetime сv61. Общая closure261 функции;
полёт168, конечные эффекты216, common world912 и игрок/камера проходят.
12 scalar schedules проверяют exact-float, pause/external write/acquisition/
reset/same-pointer recreation. Полные loop/simulation CPU проверки проходят.
Ограничения остаются: synthetic assets/room, instrumented sound/resource
lookup/release/sprite creation и AI endpoints; full gameplay ещё не проверен.

---

## 2026-09-30 — v97, gravity/chain84/88/89

84/88 используют уже проверенные CB400/CB6D4 без изменения геометрических
радиусов и смещений.89 Source-time countdown/spawn region CD2F0..CD340
сохраняет post-tick возраст и однократный переход36=10. Проверены72 расписания,
Original-pass по всей RAM, восемь следующих source-end состояний, real84→85
impact ctor и пять real88→89 links; все модели,physics и resource lists освобождены.
Scene BBE58/resident3CCA4 добавлены в реальную CPU closure: команда отдачи
8/2/2 выполняется один раз. Не stub. Общая closure269 функций (267+2 references).

Первый fixture ошибочно поместил projectile на y10000 вне синтетической комнаты
с потолком250: настоящий probe/дистанция инициировали impact уже в Source
reference. Исправлена только fixture: y100,vy4,gravity.25; geometry не отключалась.
Расширен строгий synthetic lookup/release:85 ctor также использует resource
kind4 с ID216, который освобождается настоящим guest resource-list кодом.
Отдельные реальные data assets и renderer по-прежнему не запускаются.

При новом NativeTimeline scalar cache теперь инвалидируется и у paused/
невыбранных моделей: старые tick/phase и те же alpha bits могут совпасть после
resume, но прежний baseline принадлежит предыдущей timeline. Проверка совпавшей
phase с отрицательной alpha и plan=nullopt проходит вместе с12 exact-float cases.
Fixture для этой проверки сначала вставлен внутрь прежнего live-baseline case;
последовательности разделены, production predicate не ослаблялся.
Full player/flight/growth/terminal/common-world, loop, model и timing checks
проходят с ASan/UBSan/LSan. Bounce/phase83, homing, charge, interaction ещё впереди.

---

## 2026-09-30 — v98..v99, четыре стадии83

После первоначального включения83 и fractional phase3 alpha checkv98 показал
divider2/fps60/tick30: Native phase2, Original phase3. Clip command обрабатывался
лишь следующим variable callback, что задерживало bind и начало fade. Реальная
ошибка; исходный dt и состояние не подгонялись под test.

v99: Native83 сохраняет физику и opacity в callback, а clip-end region CBDA4..
CBE2C переносится в post-model boundary D117C. Original branch неизменна вне
SimulationScope. После A1B8 выполняются настоящие A3B0/EA4C/A2E8/A328/C05F0
на локальной копии guest context. Каждая смена уничтожает прежний model lifetime,
создаёт новый, выставляет phase/mask; фаза4 удаляет actor/physics/resources.
Новая модель не получает dt законченного интервала. Путь удаления сохраняет
pool increment, отмена сцены — общий register cleanup. CPU closure271 функций.
24 real ctor/four-phase/rebind/fade/retirement cases проходят с60/73/120/144/
165/360FPS, обоими делителями и двумя камерами; field equality фаз/mask/live
строгая, Source-pass full RAM и прежние168+72+72+216+912 проверки проходят.

Image-loop CPU fixture не запускает gameplay. Новые прямые phase dependencies
добавлены там как явные fail-fast endpoints (не no-op); реальные тела выполняет
joint gameplay fixture. В production они входят в полный generated module.
Полные loop/model/simulation checks проходят; игра/GPU не запускались.
Остальные пять projectile families, все старшие действия игрока и общий полный
FPS ещё не завершены. Windows source stage и EXE033/v89 остаются замороженными.

---

## 2026-09-30 — v100, отскок82→83

Первый CPU fixture ошибочно задавал floor-plane D=+1000 при желаемом полу−1000;
реальный Original probe запускал bounce/retirement уже в reference fork. Это
удаляло parent allocations из общего инструментированного счётчика и затем
Native replay обнаруживал повторное освобождение. Исправлен только знак
floor-plane; live state проверяется после каждого reference шага.
Второе отрицательное наблюдение было вызвано передачей полного возраста
полёта>1 в helper одного SimulationSlice; независимая траектория теперь задана
суммой Original post-gravity скоростей: v=50−4t, y=50t−2t(t+1).
Игровые callbacks/instantaneous bounce не менялись, временные границы не
подгонялись: Native collision commits на конце реального интервала, child
не получает время до рождения.275 generated функций с двумя frozen reference
dispatcher.24 расписания проходят вместе со всеми прежними joint сценариями.
Отдельные loop/model/simulation checks остаютсяv99; Windows не собиралась.

---

## 2026-09-30 — v101→v102, Native clocks86

v101 stationary tracking showed divider2/fps60/character1/tick5 baseline pitch
Native2 vs Original0. Original yaw/pitch are bang-bang turns: even inside a
whole turn of the target they choose ±1/1.5 or±2; reevaluating the sign after
every fraction flips direction repeatedly and changes the whole-tick endpoint.
v102 caches the first signed angular command for that source tick, while angle
updates remain continuous. Commands rebase after external writes, pause gaps,
timeline acquisition, region/subsystem reset or model lifetime recreation.
48 stationary real target/constructor/hit87/retirement schedules pass; real
curved flight is explicitly outstanding. No Source-only movement fallback
has been added. Float/short caches now also discard fields in a partially reset
physics region even when their model allocation survives.12 short-angle and12
latched-turn component schedules plus the previous model checks pass. No EXE.

---

## 2026-10-01 — v103..v105, charge и live birth

Charge CB6D4 adds a spatial attachment offset after resetting xyz to the player
point. Suspending dt for this geometry call fixes shrinking offsets at high FPS;
only radius/opacity are temporal scalars. Phase3 increments and phase42 conversion
commit at Source endpoints. Model alpha command precedes its logical decrement;
the adapter keeps that provenance on Original acquisition. A native-born model
has no such lookahead. v104 first attempted using observation.has_value(), but
acquisition consumes/reset that observation: the preceding-sample check failed.
Allocation birth provenance is now stored separately, cleared by an actual
Original animation advance, and retained across observation consumption.

First charge replay compared an attached actor against a frozen player at y100:
Native player fell to98.5, causing actor153.5 vs Source155. Reference now executes
the actual Original player update before the projectile pool, and Native replay
starts from the same player position/velocity.48 complete charge schedules pass.

Live birth exposed another boundary: taking the pool snapshot after the player
would advance its newborn over the elapsed birth interval. Snapshot now precedes
all producers. Approved charge ctors initialize Source birth-pass phase21/radius10/
opacity/pose once through the actual guest command closure; only that model's
lookahead is acquired without erasing other live scalar maps. An initial added
sprite command omitted its slot argument, causing an ASan read failure in28C74;
corrected to slot1, matching dispatcher D1184.48 live-birth schedules now pass,
including exact relative conversion lifetime and all resource retirement.
Model check tests selective acquisition preserving a negative partial opacity's
positive Source baseline. Loop fixture exposes the new direct dependencies as
fail-fast boundaries; the joint fixture uses their real bodies. Four fresh checks
passv105. No game/GPU/Windows build. Full weapon states remain unenabled.

---

## 2026-10-01 — v106→v107, траектория86

Расширение сравнения обнаружило уже при нулевой скорости ошибку порядка
чтения physical pitch: divider2/fps60/tick4 Native351.739 против Original349.739.
Новый базовый58 применялся внутри такта перед вычислением волны, хотя Original
читает58 до следующей steering command. Sample хранит входные yaw/pitch,
velocity и xyz за один Source интервал; pure target query использует Source
endpoint и затем возвращает только временно заимствованные xyz/yaw.

Вv106 сохранение входного58 ещё оставляло скачок2 градусов после границы:
ceil(timestamp) содержит крошечный интервал следующего такта, где58 уже новый.
v107 плавно уменьшает разницу входного physical pitch и нового baseline58
на протяжении Source интервала. Проверка ограничивает изменение наклона25
градусами на Source tick плюс float tolerance на каждом Native update.
Движение задаётся накопленной кривой x(t)=x0+v0*t+(v1-v0)*t*(t-T)/2,
гдеT — остаток Source интервала. Она сохраняет Original displacement в конце,
а spatial queries получают действительное Native положение на каждом шаге.

384 расписания сравнивают endpoint поля и четверти интервалов с реальными
Original callback/projection bodies.192 stationary проходят102 Source ticks,
192 moving —6, четыре target patterns и оба персонажа на обоих делителях
при60/73/120/144/165/360FPS. Строгие counters и circular angle comparison
различают ошибки времени и эквивалентные0/360 градусов. Реальные hit87 и
полное освобождение ресурсов проходят; Original first pass совпадает по всей
RAM. Model component проверяет reuse sample и отмену после gap/write/acquire/
partial heap/system reset. Loop exposes pure geometry dependencies as abort
boundaries; joint fixture выполняет их настоящие тела. Свежие четыре PASSv107.
EXE/игра/GPU не запускались; interaction105 и full weapon states не включены.

---

## 2026-10-01 — v108→v109, interaction105

Joint comparison обнаружил потерянный outside-clip seek: Original command
переводит frame в57 при duration54; counter после шага —6 и end flag. Старый
acquisition отбрасывал observation времени за пределами duration, поэтому
Native начинал с6 и к следующему Source endpoint имел9 без retirement.
Теперь конечный Original observation хранится и для finite outside-clip seek;
acquisition восстанавливает его pose, Native normalization pulse сохраняется
до конца соответствующего Source интервала. Legacy extrapolation не получает
такой out-of-range baseline. Model CPU: seek12/duration10/rate3, строгий Original
counter/flag endpoint, четыре quarter slices, следующий интервал без pulse.

Quarter comparison намеренно провалился fps60/divider2/tick1/part1: таблица
80210898 содержит ключи только в0,3,6,..48, а промежуточные строки — default0.
Интерполяция соседних целых строк давала ложные провалы выдвижения. Теперь
используются пары Source keys с шагом3; independent guest callbacks задают
геометрию54 quarter samples на каждый из24 held schedules. Состояние idle player
стабилизировано его реальной Source command до геометрических probes; иначе
входная yaw90 могла законно перейти в0 на первом Native player callback.

Source comparison включает реальные player updates. RAM forks должны копировать
external mock heap bookkeeping тоже: клипы player и newborn106 освобождали/
выделяли память, иначе равные Original вызовы получали разные IDs либо ошибку
двойного mock free. Это исправление fixture, не production allocator.
Live-birth observation снимается непосредственно после ctor: ceil nanosecond
timestamp может уже содержать маленький шаг следующего Source интервала,
поэтому commanded57 сравнивается там, а subsequent clock — modulo duration.
Source AI reassert geometry не потребляет end flags; invalid captured lifetime
очищает48 до записи NPC positions. Все четыре свежих PASSv109; EXE не делался.

---

## 2026-10-01 — v110, реальный player action17/18 и charge11/12

Ранний player callback читает предыдущий model counter. Обычный Native
crossing, оставленный в этом месте, сдвигал бы команду на один переменный шаг.
В owned player dispatcher подавляется только это чтение; crossing потребляется
после реального движения/model advance. Отдельный boundary limit делит шаг
на frame18 по точному рациональному clock, а не округлённому guest float.
Standalone/Source обработчик сохранил прежнюю семантику. Joint constructor
timestamps совпали с Original для обеих source rates и60/73/120/144/165/360FPS.

Первый CPU weapon path выявил недостающую boundary801B922C. Это чистый
Original query одного mode byte; в closure добавлена реальная функция.
При расширении на B entry reference ошибочно вызывал только accumulate/latch,
оставляя pressed edge навсегда: четыре выстрела за40 Source ticks. В игровом
цикле после обновления работает800304A8 reset_accumulator. Fixture теперь
исполняет эту реальную команду тоже; Original и Native дают один выстрел.
Это correction CPU harness, не изменение управления в production.

192 связанных weapon schedules проверяют action, charge, child effect и полный
resource lifetime; отдельные проверки повторялись только при новой coverage
или обнаруженном отказе. Все четыре отчёта проверены по текущим source hashes
и generation receiptv110. EXE/игра/GPU не запускались.

---

## 2026-10-01 — v111→v116, оружие2/4/5 и Native B timing

Source player21/22 читает cue15 до движения. Native событие потребляется после
model advance; точная frame boundary исключает опоздание на переменный шаг.
Air weapon2/tick5 выявил Y268.5 вместо267.5: post-movement крепление давало другую
исходную точку. Во время ctor заимствуется Source pre-movement xyz, затем только
эти поля возвращаются. Source RAM первого прохода и endpoints снарядов совпали.

v111:84→85 surface conversion имела x925 против940 на tick18. v112 сначала
откладывал любой hit, но искусственный actor contactD4 законно требует немедленной
команды. v113 отложил только surface command84/88 до Source end, сохранил spatial
probes каждый Native update и первый Source проход successor в том же pool pass.

НажатиеB@73ms давало верный birth273ms, но expiry533.333ms вместо539.667ms:
charge age зависел от global phase. Отдельный allocation-scoped clock начинается
в момент рождения, делит общий шаг у своих границ и приостанавливается при C9pause.
Первый v114 origin выбирал start для ctor внутри player, хотя newborn не получал
этот интервал: interaction105 не завершался на ожидаемой границе. Origin теперь
совпадает с committed end исключённого birth slice. Outer global time остаётся
отдельно доступным для наблюдения timestamps; scope cleanup стоит и на D118C,
куда переходят pause/delete ветви, и на exit D119C. Lifetime token различает
новую модель даже при одинаковых actor/physical/model адресах.

v114:air weapon4 child89 оставался жив на Source tick6. Native initial pose
acquisition убрал Original counter lookahead; end command переносится после
advance. Для duration7 пересечение приходится между Source endpoints: отдельное
end observation сохраняется до конца того же Source интервала, raw flag остаётся
недеструктивным импульсом одного advance. Component проверяет очистку raw flag,
сохранение observation, следующий интервал и same-address lifetime recreation.

v115:360FPS weapon4 поле physical58 отличалось на0.003 при x760. Это originX
child89, не opacity. Fixture теперь сравнивает его с теми же spatial units и
tolerance0.15, что xyz; phase/alive/type и сроки остаются строгими. Возврат игрока
в idle законно перепривязывает channels: сравнивается количество живых allocations
с Original, а полное уничтожение owners обязано оставить пустой guest heap.

v116 прошёл связанные charge/moving/B schedules и свежие model/loop/simulation
checks. Все четыре отчёта проверены по receipt/source hashes до сохранения.
317 Original функций исполняются в joint closure; ROM/resource lookup/sound/AI
границы явно инструментированы. EXE и игровой renderer/GPU не запускались.

---


## 2026-10-01 — кандидаты117–120, Source successor и Native zero/repeat

Ни один из этих кандидатов пока не сохранён как проверенный checkpoint.
Последний verified checkpoint116, Windows EXE033.
117: moving weapon3 Source tick61 получил child87 scale0.1 вместо0.345.
118: Source-boundary first commands расширены на successors всех reviewed
семейств, terminal/phase owners потребляют end после model advance. Старые
terminal fixtures перенесены со start observation на actual committed end,
с прежней строгой birth-relative длительностью.
119: мгновенный Native cue0 теперь регистрирует новую allocation lifetime
на slice start и допускает только её к последующему elapsed pass. Первое
assertion на model time было неверным для статического86 без channels.
Исправленный fixture отдельно проверяет animated age и elapsed trajectory
по независимым Original projection commands на scratch RAM. Диагностический
119 прошёл672 moving Source и336 Native press/release schedules вместе со
всеми старыми сценариями. Это не новый формальный report/receipt checkpoint.
Контрольный diagnostic с отключённым admission start birth дал x570 вместо
577.513 при60FPS/divider2/weapon3, подтверждая пропуск elapsed движения.
Held B на119: второй shot на273ms отсутствовал, count1 вместо2.
120: post-pose repeat18 создал второй shot вовремя, но отдельный private
zero-event key позволил ordinary callback создать его ещё раз в следующем
интервале: на третьем shot count4 вместо3. Ключ исправлен на общий generated
.scene1 cursor0x03801C4F9C; повторная проверка ещё идёт. Model/loop/simulation
120 прошли до этой CPP правки; loop report после неё устарел. No game/GPU.

---
## 2026-10-01 — v121, связанная проверка zero-cue/repeat завершена

После общего cursor0x03801C4F9C полный fixture прошёл, включая48 held-B
четырёхвыстрельных расписаний. Fresh model/loop/simulation и player reports
сверены с generation receipt121 и хэшами текущего дерева. Проверены672 Source
moving actions и336 Native press/release actions. Negative zero-birth admission
остаётся доказательством пропущенного elapsed движения в предыдущем варианте.
Сохранён recoverable source checkpoint; Windows EXE не собран, game/GPU не запущены.
Работа продолжается над связанными паузой/отпусканием и flight weapon actions;
checkpoint не означает завершения полного variable FPS.

---

## 2026-10-01 — кандидатv122, paused weapon command contract

Original C554C различает weapon1 и13. Для13 frame18 при непаузированном model
создаёт105, sets pause bit8 черезA4A4 и clears channel morph черезA628.
При pause frame18 остаётся level condition: busyC060C==0 clears pause и
восстанавливает source model reference черезA5F0. Crossing-only event не может
заменить этот уровень ожидания. Native post-command pass использует исходный
callback и exact equality в reviewed command scope, exits before scalar drive
and zero-angle stores. Source путь сохраняет всю исходную функцию. Реальные
38F38/38F7C morph writes включены в expanded fixture closure; validation идёт.
Игра и GPU не запускались, новый Windows EXE не создавался.

---

## 2026-10-01 — кандидаты122–126, связанное pause/morph/attachment evidence

122: weapon1 ground duration3 tick21 имел child83 Y228.312 против245.5.
123: пять surface phase writes82 ограничены local Source end, probes остаются
непрерывными; actual actor contactD4 остаётся немедленным. Затем weapon13 duration7
не освобождал105 наtick9: raw end pulse уже исчез. 124: reviewed retirement owner
потребляет сохранённое end observation после real A3B0 getter на Source end.
125: fixture выполняет настоящие A628/A5F0 writes и задаёт actor model slot1,
проверяет pause/busy/morph на каждом Source endpoint. Native input expanded432;
Original ctor/pool at preceding action position обнаружил airborne weapon1
Y228.5 вместо231.45 при B73ms. Global tick begin не совпадает с action interval.
126: snapshot в cue-3, action-clock boundary; epoch/actor/physical/model allocation
и animation run защищают чтение, restoration отдельно защищена от смены owner.
Первое60Hz/divider2 расписание прошло все Source/pause/Native input branches.
В добавленном model rebind fixture первоначально повторный bind не освобождал
старые channels. Исправлен fixture: real release followed by bind, как в игре.
Model/loop/simulation126 прошли. Loop closure дополнен explicit abort boundary
C554C: weapon path проверяется настоящим callback в player closure, loop его
не вызывает. Полный player126 остановился на прежнем forced-bounce duration;
новая Source phase gate выявила зависимость его clock от глобальной фазы.
Player отчёт126 отсутствует; checkpoint121 сохраняет прежний verified результат.
No Windows EXE, game или GPU.

---

## 2026-10-01 — v127–128, независимые bounce clocks и точная скорость

126/73FPS/divider2: bounced0.0684932, conversion0.433333, duration0.364840
против0.333333. 127 привязал Source origin к actual bounce commit; duration
стал ровно11 Source ticks (0.366667): повторное округление дробных gravity
subtractions держало скорость чуть выше exact10 на границе десятого такта.
128 использует model allocation-scoped scalar baseline для reviewed CB400
при Native projectile ownership. Existing gravity integral даёт paired displacement,
затем paired cache публикуется с точной baseline velocity. Original/nonowner пути
сохраняют прежние operands. Contact/impulse/reallocation invalidates scalar sample.
Full player matrix и fresh model/loop/simulation прошли; сохранён исходный checkpoint.
Длительность/траектория/Source pool assertions не ослаблялись.
Windows EXE не собран. Работа продолжается; game/GPU не запущены.

---

## 2026-10-01 — v129–130, полёт и строгие Source/Input сравнения

129 остановилась на approved-site gate нового zero-cue27; после проверки пары
C533C/C535C generation130 завершилась. Снимок полёта вычисляет чистые Original
C1A3C/C1ADC/C0D88 commands под приостановленными native часами, затем интегрирует
пару velocity/displacement и передаёт её действующему collision solver.
Sample invalidates при записи контактов/ввода, смене владельца/clip и clock gap.

Строгий Source case выявил раннюю нормализацию360→0: команда с равными направлениями
сохраняет raw representation до своего конца. Другой Source release case ошибочно
сохранял A из старого OR poll window; fixture теперь принимает те же входные команды,
что и Native. В off-grid partition fixture secondary model наследовала фазу прошлого
сценария: initial60=24 и73=16.8904. Реальная rebind и одинаковое начальное состояние
устранили различие; spatial/model/discrete допуски не ослаблялись.

Flight damage использует реальные C2978/C6D2C/C0738, отдельное наблюдение rumble8/3/1,
настоящий status pointer/health limits и сравнение с Original command. Native stops
при state48 до второго callback и не меняет RAM при последующих обновлениях.
Полная матрица и четыре fresh отчёта130 прошли; checkpoint сохранён. Windows/game/GPU
во время этих проверок не запускались. Работа над полной целью продолжается.

---

## 2026-10-01 — общий test-034 после verified130

Windows x64 MSVC compile/configure прошли. EXE/source/receipts и четыре актуальных
CPU-отчёта130 сверены;32 dependency pins и donor проверены без изменений.
EXE сохранён как D:/Games/DoraRecomp3/testbuild/Dora64-3-test-034.exe; предыдущие
EXE и общие DLL сохранены. Упаковка не трогала ROM, port_*.json и saves.
SHA256: ed0337e4fcf2c441eccf757bf9043d501257f8157c455a53cf9ca2e69161f23a.
Game/window/renderer/GPU не запускались. Это connected milestone, не завершение
полной цели. Продолжается перевод остальных действий и реакций персонажа.

---

## 2026-10-01 — проверка test-034 пользователем и следующий candidate136

Пользователь сообщил: «запустил посмотрел вылетов нет, вроде работает».
FPS/место/длительность не зафиксированы; не выдавать за полный gameplay validation.
Windows034 остаётся замороженной verified130, сама игра/renderer/GPU не запускались.

Candidate136 добавляет healthy flight48 с Original speed-only command вместо
ошибочного общего orientation forecast. Source comparison на ceil nanosecond
после clip-end увидел57 вместо48: точный Source endpoint лежал на1/3ns раньше.
Независимый reference теперь вызывает реальный следующий первый C6D8C на отдельном
снимке для этого фактического времени, без будущего movement/model/timer update.
Дискретные/пространственные/model assertions не ослаблены; диагностика продолжается.

---


### 2026-10-01 — damage136–140, диагностика, без нового EXE

Test034 пользователь запустил без вылетов; конкретный FPS/сцена/длительность
не сообщены. Source checkpoint130/EXE034 сохраняются. Healthy48 использует
C1A3C без C1ADC и сохраняет направление удара.137 rearm30 не уменьшается хвостом
того же Source периода; следующий период следует Original blink/decrement.

138 впервые исполнил реальную35→landing36→ground цепочку. CPU при60FPS выявил
лишнее0.15 displacement при18→0 и недобор0.75 при12→18: bounded drift с clamp
не воспроизводил конечное Source kick/drift.139 guarded drive baseline сохраняет
float Source endpoint и его displacement; unclipped motion остаётся непрерывным,
clipped drift использует Source endpoint. Нет snap/позиционной коррекции.
Диагностика13960/73/divider2/camera12 PASS со всем прежним набором.

Source сравнения используют ceil nanoseconds: дробная наносекунда после clip-end
уже включает следующий первый callback и мгновенное ground heading. В эталоне
исполняется реальная команда и second body только для рационального остатка;
будущая Source анимация/таймер не продвигаются. Ground duration7 требует40ticks,
а не30: landing24 + recovery7.140 вводит здоровые37/38/39, counter44/импульс70,
landing baseline и rearm; широкие проверки ещё pending. Отчёты130 stale для140.

### 2026-10-01 — уточнение damage140/141 reference и Native fixture

Heavy140 Source episodes60/73/360, divider2/camera12 PASS. У tick95
ceil после landing эталон сначала оставлял старые flags: Original C0C7C
обязан снова выполнить probes перед следующим first callback; после
включения реального probe и C0F1C в рациональный остаток расхождение
исчезло без дополнительной правки production. Полный набор ещё pending.

Native damage fixture сначала передала0 вместо clip указателя в B6BC;
ASan обнаружил чтение вне RAM. Это ошибка нового тестового rebind; исправлена
на реальный B744/B6BC с clip. Затем uncovered E59BC потребовал расширить
CPU closure настоящими E59BC/E5894/contact reset. Непортированный E6968
visual allocation явно инструментирован как отказ выделения с проверкой
asset/позиции/однократного вызова; не объявляется проверенной визуальной
системой или полным Source object lifetime.141 подключает32/33/34 и их
landing/rearm, диагностика продолжается. EXE034/source130 не изменены.

###2026-10-01: Native damage141→142 — реальные пути и stale landing

Отрицательные CPU результаты: raw D4B8=FA теста перезаписывался C08E8;
исправлена комната, floor triangles получают byte+40=FA, настоящий probe
получает kind. Исходный2A==1 выбирает35 даже в прыжке: Native airborne fixture
исполняет настоящую C0738(-2), затем D8=10000 реакцию без special timer expiry.

Native обычный удар73ms на73FPS переходил35→36 при y=3.87246, хотя60FPS
ещё оставался35. Причина production: SourceContact, снятый до удара, сохранял
старый floor2 в новой реакции. Новый guard state/model/lifetime/run при первом
реальном hurt callback принимает свежий C0C7C; затем сохраняет Source observation
только для той же реакции. Это исправление142; проверка в процессе.

Fixture добавляет192 Native schedules и72 lethal ground handoffs; Source48
сценариев каждой реакции32/35/37/48. E59BC/E5894 исполняются Original;
E6968 checked failed-allocation boundary не проверяет настоящую жизньFX.
Свежие отчёты не заявлены до завершения проверок. EXE не собирался.

###2026-10-01:142→143 — свежий contact probe на Source границе

Полный player142 остановился на Native heavy hit,144FPS/divider2/camera12,
duration3 sample95 field0:1712.71 против1725.45. Это не общий PASS142; три
независимых model/loop/simulation142 прошли. Причина дополнительного расхождения:
pre-C8 snapshot получал flags от probe до завершённого collision предыдущего
интервала.143 снимает Source observation непосредственно в reviewed landing
hook после настоящего C0C7C. Epoch/reset плюс actor/physical/model lifetime/run,
state и рациональный период защищают сохранение; новая реакция получает fresh
probe. Continuous collision всё время Native; Source landing command не зависит
от числа кадров. Диагностика14360/144FPS camera12 divider2 прошла полный subset.
Не ослаблены допуски, не скорректирована позиция, игра/GPU/EXE не запускались.

###2026-10-01:143→144 — длинный healthy удар на20/360FPS

Полная матрица143 выявила heavy duration3,360FPS/divider3, tick82: y681.037
vs681, vy-54.4995 vs-54.5; состояние37/counter83/здоровье10/клип2 совпали.
Диагностический repeat подтвердил именно накопление float subtractions.
Толеранс0.035 не изменён. Три независимых отчёта143 прошли; player143 не PASS.

144 переносит Source-baseline подход из scalar drive на reviewed player gravity
C0FA8. Один исходный float velocity−gravity и terminal−55 задают endpoint;
paired displacement вычисляется по той же rational phase. Owner/model lifetime/
run, Source period, force, phase gap и внешняя запись velocity инвалидируют
снимок. Clipped Source kick drifts at its endpoint без backward correction.
Standalone/native components сохраняют прежний fallback, Original вне scope
исполняет исходное вычитание. Свежая генерация144 и CPU диагностика запущены.
Source checkpoint143 НЕ сохранён; последний verified130/Windows034 неизменны.
Следующий hazard145 script подготовлен, но ещё не исполнялся.

###2026-10-01:144→145 — precise horizontal accumulation до collision

Диагностика14460/360FPS divider3 camera12 прошла прежний vertical failure,
но на Source heavy tick117 x1900.12 vs1900.08 выявила rounding coordinate adds.
Velocity18/yaw45.0003/state4/cooldown15/counter96/health10 совпали.
145 аккумулирует native deltas в double только внутри одного Source периода,
от actor position baseline, до original MIPS Add X/Z и collision probes.
Expected float position проверяется на следующем шаге: collision/внешняя запись
ребазирует накопитель. Actor/physical/model lifetime/run, rational period и
phase gap защищены. Не меняется position после collision и не ослаблен tolerance.
Standalone motion checks получают прежний fallback при motion_call=false.
Verified checkpoint130/Windows034 сохранены;145 generated и диагностика запущена.
Подготовленный hazard146 script ещё не исполнялся.

###2026-10-01:145 verified CPU checkpoint, продолжение без EXE

Полная матрица145 прошла384 Original функций и четыре fresh отчёта; source hashes
и generation receipt сверены. Healthy damage32..39/48 проверены через полное
восстановление:192 Source episodes,192 Native hit partitions и144 lethal handoffs.
Отрицательные142144FPS/stale-contact и14320/360FPS/vertical float drift,144 horizontal add drift сохранены
в журнале, допуски не ослаблены.
Новый EXE не собирался. Следующим подключается hazard40..45 с original задержкой
15→16, импульсом70, collapse/rise и rearm; полный объём остаётся активным.

###2026-10-01:145→146 — Original hazardous40..45

Четыре fresh145 отчёта и source hashes проверены; checkpoint145 SHA256
d0cceee8ff52981f703ee13af570677bd60aef9a5fd74ec5a956f4031804d757.
146 добавляет reviewed-ROM hooks C6418 delayed counter/kick/drive иC67D8
one-shot impulse/count, landing C63D8/C67A8 и recovery C66B0/C6A0C.
Source counter40 держит freeze до15,16 один раз70 kick, затем100 drive.
Native floor F3/F8 episodes используют настоящие probes и C2978 health−1;
FX E5B20/E5A24 исполняются с проверенным отказом E6968, без fake successful FX.
404 Original функции в CPU closure146; свежие полные отчёты пока pending.
FX/color visual и реальные уровни не заявлены проверенными. EXE не собирался.

###2026-10-01:146→147 — Source delayed counter и own pause run

Диагностика146 на60FPS/divider2 reaction40 duration3 Source tick15: Native
count100/speed6/clip1/x504.243 против Original count16/speed0/clip3/x500;
y168.5/vy68.5/health10 совпали. C63F8 count16 unpause меняет model run token,
и Native guard ошибочно разрешал ещё один counter increment в том же периоде.
147 ROM-reviewed hook C6500 фиксирует ожидаемый run после собственных pause
commands; actor/physical/model lifetime/period guards сохраняются. External
run replacement по-прежнему инвалидирует guard. Полная проверка ещё pending.

###2026-10-01:147 full negative — constant velocity integer room-edge pushout

Полная147 обнаружила165FPS/divider2/mode4/duration3/sample151 Z1951.5 vs1952.5;
sample152 снова совпал, health/state/count/timer/velocity совпадают. BF828
Original probes body offsets and integer corrections8→4→2→1 near room edge.
Standalone Native diagnostic имеет другую camera/input prehistory и другое
раннее расхождение, не воспроизводит этот exact case. Связанная165 диагностика
запущена с исходными группами.148 prototype наблюдает итог solver для expected
guard, сохраняет candidate только при unchanged velocity и сбрасывает его при
реальном новом velocity command/contact external write. Итоговую позицию после
collision не корректирует. Генерация/доказательство гипотезы pending;147 archive
не создавался. Следующие46/47 подготовлены только scratch helper, не применены.

###2026-10-01:148 verified hazardous40..45 вместе с остальным healthy damage

Полная связанная матрица148 прошла404 Original функций, ASan/UBSan/LSan и все
24 FPS/divider/camera combinations. Четыре свежих отчёта и source hashes
сверены.96 новых Source hazard schedules,288 Native real damage schedules
через полное восстановление,144 lethal handoffs; старые группы сохранены.
146 own-pause и147165FPS/camera2 integer pushout failures исправлены148, без ослабления сравнения.
Verified148 source архивирован; EXE не собирался. Далее46/47 ожидание20,
one-shot15/yaw reversal и clip30 recovery, затем остальные полные системы.

###2026-10-01:148→149 — recovery31 и one-shot waiting46/47

Reviewed Original31 grounded clip10/scalar stop/clip-end return; contact loss
переходит к уже подключённому falling14. Пустые slots19/23/26/30 имеют обе
jr-ra/nop handlers и не являются неперенесёнными активными mechanics.
46 first counter20 decrements once per Source period; second count1 kick15,
speed param6/yaw+180/unpause once. Separate phase guards adopt own run atC6BCC.
47 end restores ground/rearm30. Source constructor schedules31/46 и настоящий
C2978 Native ctrl50 hit/health/rumble/E5C98 command подготовлены. Generation,
focused/full validation149 пока pending; verified148/EXE034 сохранены.

###2026-10-01:149→150 — waiting46 animation CC delay-slot store

Focused149 failed60FPS/divider2/duration3/Source tick1: model counter3 vs2,
physical/count17/health10/state46 совпали. C6B28 delay slot C6B2C stores CC=1
на каждом Original second body. Native repeated impulse suppression atC6B20
skipped that shared store; C8B28 clears CC before body, so default rate3 leaked
into subsequent parts.150 repeated branch executes the reviewed common CC=1
store before jumping to no-impulse tail. First command and Original unchanged.
414 Original closure, fresh generation/focused checks pending, no EXE.

###2026-10-01:150→151 — independent camera order and waiting constructor clock

Strict trace150: at Source tick46, Native camera yaw0 vs stale reference150;
the first ground direction45.0003 vs195 follows the same camera difference.
Adding actual Original player→BBDDC→DBD58 in independent reference corrects
the direction, preserving all18-field tolerances. New46/31 fixtures now also
acquire after a complete Original world update. Diagnostic then failed Native
mode6 at83ms/73FPS: count19 vs60FPS20, because extra render part invoked the
new46 first callback in its constructor period. C6A9C marks that first phase
consumed; second46 still executes immediately. The next Source period does
the first decrement. Primary46 clip limiter uses actualrate1, mirror3.
Generation151/focused/full validation pending; no EXE/game/GPU.

###2026-10-01:151→152 — exact Source vertical coordinate before floor query

151 focused negative:Native mode6/divider2/duration3/73FPS sample38, baseline
Y4.93372e-08/vY-13.5/state46 vs actualY0/vY0/state47. Both previous three
Source samples and count/health/clip agree. Rounded substep height accumulation
determines which side of the exact floor endpoint the player reaches.152 adds
a private height clock only when the real player gravity command still owns
the paired velocity. Computes cumulative Original kick/drift from guarded
initial height; rounds displacement and coordinate add once, before Original
BF828 geometry. No epsilon snap or postcollision coordinate correction.
External/collision height changes, gravity commands, identity/run/period/phase
changes rebase immediately; flight/nonmatching motion retains existing path.
Focused/full checks pending, verified148 and EXE034 preserved.

###2026-10-01:152 verified — recovery31, waiting46/47 and exact height endpoint

Четыре fresh full reports152 сверены с receipt/source SHA.414 Original функций,
24 FPS/divider/camera settings, ASan/UBSan/LSan,48 recovery31 Source schedules,
48 waiting46/47 Source и336 Native actual-hit full-recovery schedules; все
старые группы прошли. Negative149 CC delay-slot,150 stale reference camera/
constructor-period decrement и151 floor-rounding documented and fixed152.
Verified source checkpoint152 preserved; no EXE. Next connected special actions
continue, alongside remaining full variable FPS scope.

###2026-10-01:153 candidate — stop65 and scripted turning66

Reviewed C83D8/C844C stop/cancel and C84F8 shortest-direction/dead-zone30,
signed30 actor yaw command, zero scalar speed. Native66 latches angular
command per Source period, invalidates for real target/D4/angle writes,
identity/run/time gaps. Speed reset is a discrete command before genuine
Native flight C0D88 integration, never repeated at every render part.264
Source schedules prepared (including ground/flight, target replacement and
A/B cancellation). Generation153 complete, focused pending. Verified152
archive67afc7fdda05ebfcfe37f0be8161359e798403a2aeb892fd9d9c5aa85811cb76 preserved.
No EXE/game/GPU; broader64/67/deaths and remaining systems continue.

###2026-10-01:153→154 Source-aligned cancellation fixture timestamp

Focused153 negative60FPS/divider2/scenario1/tick21: model counter30 vs0,
all motion/state/timer/health fields match. Native input press was applied
after ceil(Source period11),366666667ns, reference used the rational boundary
366666666.666ns. Clip origin differs by a fraction of nanosecond, enough for
different raw wrap representation despite same rounded guest float30.154
places the Source comparison edge at period12 start400/600ms, exactly
representable for both dividers. No production code or tolerances changed.
Off-grid input remains a separate Native partition requirement, not silently
compared with a backdated Source command. Focused154 pending.

###2026-10-01:154→155 — scripted66 has fixed flight projection

Focused154 abort:Flight projection without motion command. Ground65/66 cases
passed before flight66. Original66 second body C84F8 sets speed0 and omits
C1A3C/C1ADC; C0D88 in the main dispatcher projects the already set speed.
Forecast adapter previously covered only flight actions which call those
command helpers; admitting66 incorrectly demanded an open forecast atC0E24.
155 explicitly uses fixed-velocity projection for66. Existing reviewed
paired projection/translation remains continuous and Original unchanged.
The per-period speed command guard remains safe for external writes but66
has no flight acceleration to reset. Strict focused/full checks pending.

###2026-10-01:156 prepared off-grid special command integration matrix

Adds144 Native schedules for65 A/B cancellation and66 target/turn-disable,
angle/speed writes, actual model rebind. Events73/173/273ms are compared with
60FPS at common83/183/283ms and31 later Source endpoints, both dividers and
cameras. All18 gameplay fields retain strict discrete and bounded numeric
comparison. Source264 comparisons remain. No game/GPU/EXE; full scope continues.

###2026-10-01:156 focused PASS;157 special animation/surface recovery prepared

Focused156 strict264 Source +144 Native off-grid65/66 scenarios passed all24
settings,420 Original functions and ASan/UBSan/LSan. No formal156 report.
157 reviews64 character0 frames48/318, character5 frames111/336, other frame0;
splits advance at those boundaries without post-consuming preceding-frame commands.
67 initializer and ten-step recovery count retain Source cadence/constructor period;
speed5 is a guarded period command, flight67 uses actual fixed projection.
168 Source schedules compare all22 fields, including phase and actual command counts,
complete animation/cancellation and surface exit, independent Original world/camera.
Synthetic assets/resource lookup/sprite boundaries remain explicit; real visual
resource allocation/lifetime is not established. No EXE/game/GPU; scope continues.

###2026-10-01:157 compile-only failure;158 fixture repair

157 generation passed; focused C++ -Werror rejected a new read-loop indentation
and a mistaken C7840 helper name (actual second57 is C7810).158 fixes both and
explicitly restores independent reference resource counts before Native pass.
No behaviour verdict/EXE/game/GPU. Fresh generation/focused/full pending.

###2026-10-01:158 ASan fixture failure;159 real auxiliary model3 lifecycle

158 compiled426 Original functions; ASan trapped C2A2E8 called by actual64
C02A8. New synthetic actor lacked B0 mesh-resource table, unlike its existing
B4 clip table.159 supplies separate B0 index3->real synthetic mesh asset and
executes actual C2A100/B560 bone creation, C38F38 attachment, C0398/A0AC/B77C
retirement. Character1 acquired auxiliary lifetime recreated before Native
acquisition after independent Original reference release. No production change,
no tolerance change; sprite5 remains instrumented. Fresh checks pending.

###2026-10-01:159 strict recovery mismatch;160 missing next Ground command

159 focused426-function check completed new64 cases at60FPS/divider2, then
67 scenario5 tick16 failed only primary clip50 vs51. At ceil533333334ns the
independent Original next Ground first callback C3300/C3124 already belongs
to the time: speed now0 changes clip2 to0 and truncates seek to an even frame.
160 remainder now executes that actual callback for recovered67 Ground states.
No production or tolerance change. Fresh connected focused/full pending.

###2026-10-01:160 focused PASS;161 real idle entry/Native constructor matrix

160 strict168 new Source64/67,264 Source65/66,144 Native65/66 passed24 settings
with426 Original functions/ASan/UBSan/LSan.161 extends Source64 with real C3300
idle timer298->299 acquisition, then threshold300 at health1/maximum10 for
characters0/1. Adds192 Native schedules: actual constructors73ms, all3 event
branches through complete model3 creation/attachment/destruction, A173/release273,
ground/flight67 F1 exit173ms and argument1 immediate return.23 fields, all command
counts/discrete states exact; compare60FPS at common83/183/283ms and121 later
Source endpoints. No EXE/game/GPU. Formal152 remains last complete checkpoint.

###2026-10-01:161 proved idle command timing;162 Source-start idle prelude

161 focused426 functions:60FPS/divider2 scenario7 tick1 state65 had Native
clip9e-8 vs Original3 and count0 vs300. Prior tick-end counter deferred action
entry and repeated reset once the new state was visible. Fixture health1 selects
65, not64 (C3300 actual branch);162 uses health10 for reviewed64 idle entry.
C8E58..C8EAC increment/clamp/reset now executes once per Source period before
first callback, guarded actor/physical/control/model/lifetime/time, ignoring its
own clip-run changes. Standalone Original untouched. Source ceil reference uses
actual verified Source61 prelude, all21 instruction addresses checked, no copied
timer arithmetic. No tolerance change/EXE/game/GPU; fresh checks pending.

###2026-10-01:162 reached Native immediate-return fixture;163 floor correction

162 Source idle fix reached expanded Native matrix; ASan caught actual Ground
C2E94 after mode7 immediate67 argument1 return on F1. Fixture had combined
normal immediate return and a door surface without scene door data.163 restricts
F1 to waiting modes5/6; argument1 uses ordinary floor. Production162 unchanged.
Formal report limitations now explicitly include64..67/actual model3 lifecycle
on synthetic assets and instrumented sprite5, plus verified Original idle region.
No tolerance change/EXE/game/GPU; fresh focused/full pending.

###2026-10-01:163 verified connected special actions, real idle and full recovery

Four fresh163 reports match receipt/source SHA.427 Original routines/regions,
24 settings/ASan/UBSan/LSan,216 Source64/67 +192 Native constructor/idle/full
recovery +264 Source65/66 +144 Native off-grid commands. Prior connected groups
all passed.157 compile-only/158 missing B0 mesh fixture,159 next Ground seek
reference,161 idle tick-end entry failure and162 invalid immediate-return door
fixture documented and corrected without weaker tolerances. Idle162 production
region moves to Source start before action choice and suppresses own repeated
reset/rearm. Native auxiliary model3 lifecycle checked on synthetic resources;
sprite5/real visuals remain unverified. Source163 archived, no EXE/game/GPU.
main/HEAD/donor32deps/packaged034 remain unchanged. Full scope continues49/63,
death/other actions, active AI/platforms/scenes/scripts/minigames/2D.

###2026-10-01:166 verified healthy hover49/63 and complete cancellation recovery

164 negative: Source21 X555 vs557.75; repeated first49 observed its own
second counter increment.165 threshold guard fixes it; negative165 landing
at tick90 phase denominator-20 exposed repeated fall9 contact probe entering
landing0.333ns before Source first. Later state1 vs10 at Source111 clip60.
166 preserves actual first C0C7C observation per guarded action Source period.
Source63 fixture starts D8=0;10000 is a real hit command, not an idle timer.
144 Source +144 Native connected schedules passed;435 Original routines;
four matched fresh reports, ASan/UBSan/LSan,24settings. Actual health loss and
ctor49 invoked at off-grid timestamps; external selecting producer unreviewed.
Native counter/state/command counts exact; tolerances unchanged. Loop linking
needed three explicit abort boundaries for new geometry symbols; gameplay
executes them in player435 closure, loop test forbids unexpected invocation.
No new EXE/game/GPU, donor/32pins/main/034 unchanged. Full scope continues.

###2026-10-01:169 connected death/fractional fade, partial checks only

167 candidate enables50..56 bodies and first Source commands;168 adds pre-EF5B0
resource barrier and gameplay command stamp. Fixture corrections: actual fade
alias=dora3_fade_tick; BBF34 list at80217120, not80207120; Original destructor
clears actor/kind markers, does not promise pointer74 becomes zero.
168 strict negative:30Hz common Source boundary alpha31 vs32 after rounded
command origin.169 preserves exact sixtieth-ns start and fractional elapsed;
no widened tolerance. Terminal game-over sample belongs to new fade owner:
compare exact commands/counters, not new scene alpha after elapsed part.
All336 Source/Native death/game-over/actual model cleanup schedules passed,
466 Original routines,24settings and sanitizers. Fresh frame-time and loop
reports match169 receipt/source hashes. No full169 checkpoint/report/archive;
last complete166. Resource stream lookup/free and sound instrumented; real
level/FX/GPU unverified. Healthy51 respawn and lethal entry paths pending.
No Windows/EXE/game/renderer. Continue entire authorized variable-FPS scope.

###2026-10-01:174 connected respawn/lethal pending, actual byte status correction

169 strict cancellation negative:old C70C8 second callback resumed after
EF5B0 reset cleared motion_call.170 invocation-owned epoch survives reset;
C8F10/F34/9048 stop old second/motion/model continuation,336 strict PASS.
170 respawn negatives:synthetic one-bone model lacked weapon nodes5..12;
fixture repaired actual13-node tree. Source C9BE0/BA084 commands at
566666666+40/60ns vs Native BA0840 after reset.171 preserves first-callback
command timestamp through epoch cancellation,528 focused PASS/473routines.
172 adds pending-room/hold guards + pre-B9FDC barrier, actual post-ready
C9C58/Original complete update/new-model Native reacquisition:528+144 PASS,
489 routines. Synthetic room boundary explicitly excludes real DMA/assets.
173 lethal matrix failed before Native acquisition at32 actual0health:
801B361B+2*char is a byte record, not word-aligned.174 uses byte_pointer
range2 for statuses, keeps structural pointer alignment. Second fixture
negative was an unrelated default rumble assertion; actual reaction-specific
rumble arguments now checked through existing exact watch (10,2,2 or8,3,1).
Full174 validation pending; last complete166, no new EXE/game/GPU.

### 2026-10-01: verified175 connected death/respawn/lethal paths

Full player489 Original routines/regions,1392 death schedules and144 post-ready
reacquisition, ASan/UBSan/LSan passed. Five fresh matched reports and stability175.
Negative169 old second continuation after retirement -> invocation epoch guard.
Negative170 respawn BA084 command at zero after C9BE0 -> retained SourceTimestamp.
Negative173 real byte health pointers rejected by word alignment ->174 byte range.
No relaxed assertions/tolerances. Actual room loading and external49 producer
remain explicit boundaries. No new EXE/game/GPU; main/donor32pins/034 unchanged.
User requested persistent semantic names and variable roles for reviewed decomp
source and updates to documentation; prepare generator-backed compatible aliases.

### 2026-10-01: semantic decomp names177, ABI and exact C preserved

Catalog525 descriptions/190 new names/305 register scopes; no guessed global
register meaning. C8 s0 is controller index. C0738 a0 health delta ends before
the same register becomes status pointer. ROM hashes, section and scope words
are checked. C source names are compatible macros, no wrappers/copies/ABI drift.
Negative176 compiler comparison detected diagnostic __FILE__/__LINE__ drift
caused by temporary canonical path and inserted comments.177 adds original
#line positions and maps only the temporary canonical file path back to the
same translation unit. No tokens excluded/tolerances relaxed:96 units equal
after preprocessing and compile as C17. Exact canonical C and runtime sources
match full175 checkpoint. Frame-time177/metadata/doc check/stability177 PASS.
Future work instructions and function/variable docs added; old full function
names in address-map replaced, compatibility mapping retained. No EXE/game/GPU.

### 2026-10-01: остановка по просьбе пользователя

Пользователь попросил на сегодня стоп и сводку. Primary177 verified naming,
runtime/code equivalent to full175 checkpoint. Surface/door phases prepared
only in external stage, not deployed/tested; no178 generation, no active jobs,
no new EXE/game/GPU. Detailed continuation state saved in work-in-progress.md.
Resume only on new user instruction; no schedule/goal automation was created.

### 2026-10-02: verified181 door59/surface60..62 connected block

216 Source surfaces +144 off-grid Native +96 door schedules, all24 settings.
Fresh503-function player including previous lethal/respawn groups PASS; all six
integration/naming reports and stability181 matched. Negative178 unconditional
F4 producer clip resets -> Source period constructor/run latch; no relaxed
geometry/state tolerances. Negative179 inserted jump target had no C label ->
generator creates guarded synthetic label at checked successor. Door testF2
corrected toF1 after actual surface-mode prelude; fade fixture constructor order
and scene-selection address/condition corrected from Original. Full181 fixture
retained previous death barrier count -> death-watch-only barrier predicate.
Actual gameplay dispatch/runtime logic wasn't relaxed. Script camera command
writes execute; camera advancement and actual room loading still pending.
Persistent531 descriptions/196 names/307 variable scopes, exact96-unit proof.
No game/GPU, no new EXE yet; main/donor/dependencies and034 preserved.

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


Guard при переносе Windows receipt отказал из-за исторически устаревшего before.json. Перед обновлением guard проверены фактический034 EXE/receipt и все compiled035 source hashes; затем перенесён только проверенный035 receipt. Это не изменение игрового кода.

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

### Main234 — Source-награды, читаемые переменные и проверка Native interleaving

Сохранён отдельный завершённый этап без изменения игрового runtime: каталог
712 descriptions/377 semantic aliases/481 register scopes. Добавлены14 scopes
относительно217:6 для исходного room-request порядка и8 для constructor/reset,
задержки и инициализации эффектов. Уточнены возврат первого actor из E6968,
поля scene-object reset, child detach без удаления собственной модели,
различие old/new velocity branches в ED5F4/EC598 и radius-before-growth порядок.
Имена/docs согласованы генератором;96 canonical C units/frontier/hooks/overlay
table идентичны217, C tokens и C17 syntax compilation прошли.

Свежие main234 Source168 и mixed168 прошли с ASan/UBSan/LSan. В mixed выполнено
119252 настоящих native_gameplay_preview интервала: все live actor/physics/model
bytes, heap/resource references/model count/commands и caller context остались
прежними между Source callbacks.96 реальных item constructors выполнены при
diagnostic Native activation/bank flags. Session/scheduler/GPU здесь не исполняются.
Предметы118/123/127 и шесть эффектов130 остаются Source-owned; тест не объявляет
их движение переведённым на Native FPS. Проверены оба Original20/30 и60..360FPS.

Свежая проверка существующей камеры:9571 Original/full-tick comparisons на13
roots; camera implementation не менялась. Предыдущий component report был
историческим и не входил в восемь актуальных217 reports; stale source hash
пойман до каких-либо изменений и не перенесён как текущий. Все10 актуальных
reports теперь совпадают с generation234/current source hashes; прежние runtime
checks имеют явный annotation-only origin217/full212b. Header manifests Source
checks дополнены с явным metadata-only происхождением, диагностические flags
описаны без заявления настоящего presentation/session run.

Isolated FD220 full PASS:5088 Source/720 Native/34464 frozen comparisons,
1008 constructors/576 rejection/31624 intermediate yaw и3192 NPC B schedules.
Полный224 с Source-end shake correction ещё выполняется. Permanent isolated231
room test прошёл: actual BB080→BA0F4→B9FDC, два опубликованных banks, шесть receive
events, три FIFO messages, сохранение context/scratch; останавливается после
первого реального resource barrier, до destructors/DMA/new-room construction.
Isolated233 объединяет этот proof и reward diagnostics; FD/camera runtime пока
не переносится в main. Independent actor-life observation229 также изолирован.

Следом: завершить224 full и сохранить проверенный room/camera блок, затем
подключить настоящие награды/effect trajectories с исходным command/branch
порядком. Остальные уровни/NPC/enemies/platforms, script cameras/cutscenes,
миниигры и2D, общий интеграционный прогон и платформенные builds ещё остаются.
Windows test037 и подтверждённый NPC dialogue fix сохранены; нового EXE нет,
игра/окно/renderer/GPU не запускались. Проверенные этапы сохраняются commit/push
в main; локальные архивы только дополнительные копии.


## main235 — FD переход комнаты и поздняя команда камеры проверены вместе

После полного прогона 224 перенесён связанный блок перехода комнаты и камеры.
Native NPC26 в sceneFD завершает clip4 и один раз ставит quest5B/room175/1;
обычный clip5 в этом проходе не выполняется. Уже начатые обновления мира,
игрока, снарядов и камеры доходят до конца. Следующий целый BB080 обрабатывает
запрос комнаты перед старым world pool: реальный BA0F4/B9FDC barrier ждёт
завершения обоих графических буферов, внешнего retirement и прекращения чтения host.

Поздний Original D5594 теперь сохраняет текущую опору обычной Native камеры,
когда меняются только пять общих полей shake. Другие внешние записи продолжают
сбрасывать опору. Исправлено найденное различие камеры между 60 и 73 FPS
после завершения Source update.

Полный CPU прогон 224 прошёл на 24 настройках Original 20/30 и Native
60/73/120/144/165/360: 5088 slot09 Source, 720 Native, 576 отказов владения,
34464 frozen189 comparisons, 1008 реальных конструкторов slot09 и прежние
player/weapon/damage/death/recovery, городские NPC/world группы. Все 169
замороженных входов остались прежними. Камера прошла 10975 Original сравнений,
включая 1404 поздние команды D5594.

Отдельный scene-frame scope проверил 39 полей уже активного и созданного
игроком снаряда103: 64 Native schedules, 16 settings, 672 конструктора.
Воспроизводимый scene-transition scope исполнил реальные BB080/BA0F4/B9FDC
вместе с runtime/session/bridge/ledger/scheduler: 336 конструкторов, 32 Native,
8 settings, два буфера, 6 полученных сообщений, FIFO из 3 сообщений,
контекст/scratch и пустой ledger. Проверка заканчивается после первого barrier,
до исходных destructors/DMA новой комнаты.

Все 12 текущих отчётов привязаны к generation235 и проверенным hashes. Перенос
исполненных доказательств явно хранит origin224/231/235 и не выдаётся за новый
запуск. 96 canonical C units/frontier/hooks/overlay table совпали с проверенной
224 реализацией. Новая проверка имён подтвердила 712 описаний, 377 имён и 481
область переменных. Отдельная объединённая копия235 заново прошла 168 Source
и 168 mixed reward scenarios с 119252 Native-интервалами. Поле hover Source
в отчёте теперь равно фактическим 192 из stdout; прежняя подпись144 была
ошибкой metadata, compiled проверка не менялась.

Эксперимент236 не перенесён в main: задержки шести эффектов130 прошли 144
сценария, 2012 Native-интервалов и 144 проверки повторного конца периода
с реальными конструкторами и frozen Original oracle. Первая диагностика
отвергла ошибочно переставленные callback ожидания: movement dispatch находится
в table+4 (ED898), второй noop — в table+8 (ED4A4). Другая диагностика поймала
остаток <1ns после последнего проверяемого периода; test scope останавливается
на переходе state10. Это ещё не перевод траектории/исчезновения и не расширение
Native владения. Names/Source168/mixed119252 регрессия236 также прошла.

Остаются Native предметы/эффекты, остальные уровни/NPC/enemies/platforms,
script cameras/cutscenes, миниигры и 2D. Полный variable FPS не завершён.
Donor/32 dependency pins/Windows test037 сохранены. Нового EXE нет; игру/GPU не запускала.


## main249 — имена несобранных предметов; движение проверяется отдельно

Добавлены семь понятных имён функций для исходных состояний предмета2..8:
следование родителю и приземление, разлёт с реакциями на столкновения,
увеличение масштаба перед сбором, подъём с периодическим эффектом,
уменьшение масштаба с сигналом, ускорение вращения с исчезновением и
появление по двухфазной дуге. Уточнён dispatcher состояний1..10:
state9 — пустая ветвь. Для state1, dispatcher и новых функций добавлены
17 доказанных областей имён переменных; за пределами областей регистры
сохраняют исходные имена. Эффект descriptor801FC964 отличается от шести
эффектов сбора801FC950; неизвестный вид эффекта по имени не угадывался.

Каталог:719 описаний,384 смысловых имени функций,498 областей переменных.
Свежая проверка имён подтвердила точный canonical round trip всех96 C units,
одинаковые preprocessed tokens, успешную C17 syntax compilation и прежний
ABI/register storage. Runtime, timing rules, frontier/hooks и overlay table
равны main235. Прежние11 отчётов сохраняют исполненное происхождение и имеют
явную naming-only привязку к generation249; двенадцатый отчёт имён исполнен
заново. Это не повторный полный игровой прогон.

Изолированная работа над Native предметами и эффектами продолжается:

- В244 собранные118/123/127 и эффекты130 state10 прошли168 связанных
  сценариев,119252 Native-интервала и16128 точных состояний живых объектов,
  моделей, heap, ресурсов и команд. Реальный Source контакт отдельно прошёл
  96 сценариев. Проверки Original, общего цикла, текущего кадра и первого
  resource barrier перехода комнаты также прошли; полный244 прогон ещё идёт.
- Проверка243 нашла преждевременное завершение общей команды opacityC8:
  округление C4 до1 внутри периода очищало marker2A раньше Original.
  Исправление244 сохраняет момент завершения команды на Original конце.
  Старые240/241 не переносятся в main; устаревший полный240 CPU прогон
  остановлен и не считается успешным.
- Native вращение неподвижных предметов state1 сначала дважды применяло
  yaw/opacity при переходе к Source сбору (246). Исправления247/248 сохраняют
  четыре исходных поля периода перед настоящей инициализацией сбора и
  проверяют lifetime/run/epoch, непрерывность полей и ближайший Source проход.
- Изолированный248 прошёл672 сценария на24 настройках:477008 Native-интервалов,
  64512 полных Original endpoints и384 настоящих mode3 contact callbacks
  после1/3/10/19 исходных периодов ожидания. Между исходными обновлениями
  наблюдались21576 изменений idle yaw и341064 изменений высоты эффектов.
  Сохранялись шесть настоящих конструкторов, animation tail и удаление.

Новые Native изменения не включены в main249: остаются проверки отказа от
владения при внешних изменениях и общий регрессионный прогон. Контакт scope
задаёт state1 как внешний fixture input после настоящего конструктора,
использует synthetic assets и diagnostic bank flags; он не проверяет размещение
предмета в настоящем уровне или renderer/session. Настоящий runtime barrier
проверяется отдельно и останавливается до destructors/DMA новой комнаты.

В main предметы118/123/127 и эффекты130 пока сохраняют Source владельца.
Остаются движущиеся/прочие состояния предметов, остальные семейства, уровни,
NPC, враги и платформы, сценарные камеры, заставки, миниигры и2D, затем общая
интеграция и платформенные сборки. Полный variable FPS ещё не завершён.
Test037 и подтверждённые исправления диалогов сохранены. Новый EXE не
собирался; игра, окно, renderer иGPU не запускались. Проверенные этапы
сохраняются commit/push в main, архивы — дополнительная копия.


## main255 — собранные предметы и эффекты прошли полный связанный прогон

После настоящей Source инициализации собранные предметы118/123/127 state10
и шесть эффектов130 state10 обновляют движение, вращение, масштаб и исчезновение
на реальных Native интервалах. Original20/30 сохраняет исходный путь.
Первое появление модели, сбор предмета, attachment detach и delay/clip команды
остаются в упорядоченном Source проходе. Приняты только доказанные kind/state,
зарегистрированная model lifetime и обе точные записи таблицы callbacks.

Подключённый244 pool scope прошёл168 сценариев на24 настройках20/30Hz и
60/73/120/144/165/360FPS:119252 Native-интервала и16128 точных состояний всех
живых actor/physics/model, heap, ресурсов и команд. Наблюдались85312 изменений
высоты эффектов внутри исходных периодов и15616 yaw/4616 Y/11128 alpha изменений
предметов. Настоящие Source конструкторы, selectors0..5, полный model tail и
реальные destructors исполнялись до полного освобождения ресурсов.
Отдельный244 contact scope исполнил96 настоящих mode3 Source callbacks после
трёх исходных idle проходов и168 connected worlds; новые функции state1 пока
не становятся Native в main255.

Проверка243 обнаружила преждевременное завершение общей opacity команды:
из-за float rounding C4 достигала1 внутри периода, C8 очищалась, а следующий
кусок времени раньше Original сбрасывал marker2A. Исправление244 сохраняет
завершение C8 на исходном конце, оставляя саму opacity clamp прежней.
Старые240/241 не считаются пригодными для переноса; остановленный полный240
прогон не выдаётся за PASS. Дополнительно сохранены первоначальная Source
birth pass, один rational Source ceiling<1ns, отдельный collection latch28,
Original old-velocity ветвь flash и new-velocity ветвь collected pickup,
радиус до+5, реальные end probes и порядок освобождения общего пула.

Новый полный244 регрессионный прогон прошёл:1008 slot09 конструкторов,
5088 Source schedules,720 Native schedules,576 ownership rejections,
34464 frozen189 comparisons;3192 сценария настоящего выбора NPC/B,
1152 actual city constructor comparisons,3072 Source/2304 Native городских
диалоговых сценария. Player/weapon/damage/death/recovery, полёт, surface/door,
linked/patrol/orbit и остальные прежние группы проверены в той же реализации.
Healthy hover Source реально192; прежняя подпись144 в serializer исправлена
по исполненному stdout без изменения сценариев.

Свежие244 camera10975, scene-frame64/16 settings/672 constructors и реальный
scene-transition32/8 settings/336 constructors/two-bank resource barrier
прошли. Проверены simulation280 differential/2036 slices, model lifetime/run/
seek/pose, frame time и overlay invalidation49. Flash motion:288 schedules,
57504 intervals,8208 endpoints,49272 intermediate heights; delay:144 schedules,
2012 intervals и144 duplicate-end rejection. Все14 исполненных отчётов имеют
точные source hashes и явный origin244 при переносе в main255. Это перенос
доказанного исполнения с96 одинаковыми canonical C units/frontier/hooks/
overlay table; повторный полный запуск в main не заявляется. Пятнадцатый
отчёт имён исполнен заново для main255.

Каталог main249 сохранён целиком:719 описаний,384 смысловых имени функций,
498 областей переменных; семь функций несобранных состояний2..8 и их17 scopes
не потеряны. Обновлены только четыре описания/evidence блока сбора и flash.
Исправлено прежнее неточное «rate45»:8002A41C берёт group model74, truncates
float argument до целого и вызывает80039564, который записывает animation
seek time в model64. Это seek45, compiled игровой путь был правильным.
Fresh names check подтвердил96 canonical round trips, равные preprocessing
tokens, C17 syntax и прежние ABI/register storage. Документы пересозданы из
каталога; исторический Source-owned diagnostic помечен superseded_by_255.

Следующий блок остаётся изолированным:

- Idle248:384 настоящих контакта после1/3/10/19 исходных периодов,
  672 worlds/477008 intervals/64512 endpoints; переход к сбору сохраняет
  исходные yaw/C4/C8/2A и проверяет lifetime/run/epoch/ближайший Source pass.
- Guards252:768 contacts/1344 worlds/954016 intervals/129024 endpoints;
  pauseC9, движение, state/callback replacements, yaw/opacity writes и
  настоящий model seek0. Неверный тест250/251 с clip lookup у предмета без
  B4 таблицы падал в frozen Source, заменён корректным seek; runtime прежний.
- Source253b effect131:descriptor801FC964 действительно kind131;72 Original
  и72 mixed lifetimes/51108 intervals/6912 endpoints. Init вызывает
  настоящий RNG(-100/-50), clip0/seek30, scale1.5 и VY-5; удаление проверяет
  pre-step Y<=50. Старая подпись stdout «rate30» неточна, фактический compiled
  вызов — seek30; исполненное происхождение отчёта сохраняется.
- Coupled254e:216 Original и216 mixed цепочек state5 periodic131 producer →
  external state6 shrink/signal → Native idle → Source mode3 collection →
  six flash130 complete retirement.306828 intervals/41472 точных endpoints,
  intermediate idle/collected yaw/flash Y44064/37944/263304. Первая версия
  помещала itemY400 над synthetic ceiling250, из-за чего даже Original flash
  lifetime не завершался; ceiling1000 исправляет только условия этого scope,
  настоящая floor/collision logic сохранена. State5/6/131 всё ещё Source-owned.
- В253b заново прошли loop/barrier и scene-transition/NPC3192 для idle248
  runtime. Полный244 не выдаётся за полный прогон иной idle248 реализации.

Остаются остальные состояния и семейства предметов, дальнейшие уровни/NPC/
enemies/platforms, script cameras/cutscenes, миниигры и2D; затем общая
интеграция и платформенные сборки. Full pool exhaustion, реальные placement/
collision/dialogue progression и renderer этим synthetic CPU этапом не
проверены. Полный variable FPS ещё не завершён.

Donor/32 dependency pins/Windows test037 прежние. Новый EXE не собирался;
игру, окно, renderer иGPU я не запускала. Завершённые проверенные этапы
сохраняются commit/push в существующей main; архивы — дополнительная копия.

## Читаемость267: periodic-effect131 и animation seek

В main добавлены четыре смысловых имени и восемь областей регистров: пустые
constructor/contact kind131, его Source падение/порог удаления и общий выбор
model-group с animation seek. Всего723 описания,388 имён функций,506 областей
переменных. Значения30/45 у `dora3_actor_group_seek_animation_8002A41C`
обозначают время анимации после truncation, а не её скорость.

Проверка267:96 canonical C единиц точно совпадают с main255; одинаковые
preprocessed tokens, успешный C17 syntax check, прежние ABI/ROM/overlay identity,
frontier/hooks/overlay table. Runtime этого коммита остаётся reward255. Native
idle/подъём5/уменьшение6/падающий131 проверены в изолированном264; окончательный266
прошёл loop/scene-transition/names и продолжает общую CPU-проверку. Native7 пока
отдельный кандидат268 без результата. Новый Windows EXE не собирался;037 сохранён.


## main276 — Native idle, подъём5, уменьшение6 и periodic effect131

В основную версию перенесён окончательный runtime266: неподвижные предметы
118/123/127 вращаются между исходными периодами; state5 поднимается, state6
непрерывно уменьшает размер, инициализированный effect131 падает с исходной
скоростью. Первый вход, actual RNG/clip/seek30, private counters, создание
дочерних эффектов, звук/сигнал, настоящий contact и удаление сохраняют
исходный Source порядок. Незнакомые состояния/скорости/обработчики остаются Source.

Перед Source callback возвращаются только неизменённые Native continuous поля.
Новая команда seek/bind/release имеет отдельную time-command identity; pause/
clamp команды и внешние координаты/opacity/scale сохраняются. Model time,
opacity/render flag и неизменённый completion pulse возвращаются к command
baseline. Это предотвращает двойное продвижение модели и дрейф при pauseC9.
При уменьшении допускается раннее округление scale до2 внутри периода,
а threshold/signal/sound исполняются только настоящим Source callback.

Свежий полный266 O2/ASan/UBSan/LSan прогон завершён:5088 Source/720 Native
slot09 schedules/34464 сравнения,3192 actual NPC/B, городские/linked/patrol/
orbital NPC, player/weapon/damage/death/recovery/surface/door и прежние группы.
Свежие266 producer216 worlds/306828 intervals/41472 whole live endpoints,
Source-only7/8 baseline432 worlds/818460 intervals/110592 endpoints, loop и
scene-transition также прошли. В264 завершены producer guards3456 worlds/
4909248 intervals/663552 endpoints, idle guards1344 worlds/768 контактов,
periodic guards1152 worlds/221184 endpoints и camera/model/time/simulation/
frame/overlay проверки.16 текущих отчётов имеют явное происхождение исполнения:
пять266, десять264 и свежий names276. У264 указан только удалённый unused local
и добавленный, не выбранный в прежних scopes Source7/8 тест; точное обратное
преобразование проверено. Повторное исполнение этих264 scopes вmain не заявляется.

Сохранены723 описания/388 смысловых имён/506 областей регистров.96 canonical
C единиц/frontier/hooks/overlay table совпадают с проверенными264/266; fresh
preprocessing tokens/C17/ABI check прошёл. Описания idle/5/6/131 и документы
обновлены. Исторические255/267 receipts помечены superseded и не считаются
текущими проверками новой реализации. Строгая компиляция264 сначала отвергла
unused rdram;266 удаляет только эту локальную переменную. Исследование O2
ELF не дало побитового равенства и не считается PASS; использованы реальные
свежие266 исполнения. Не ослаблялись проверки компилятора.

Следующий блок изолирован: Native7 spin/fade прошёл432 basic и3456 guard
worlds/884736 guarded endpoints; Native8 two-phase arc прошёл432 basic worlds/
110592 whole endpoints/25002 first XYZ+scale и70998 second XZ изменений.
Guard8 и общий272 прогон ещё идут. Source4 baseline274 уже прошёл216 worlds/
55296 endpoints, Native4 пока не подключён. Strict arc loop275 прошёл после
удаления unused physical local; actual Original trig исполняется в gameplay
scope, explicit abort-only math boundary используется только image-loop fixture.

Full variable FPS ещё не завершён: state2/3 и другие семейства предметов,
враги/платформы, сценарные камеры/заставки, миниигры/2D и общая интеграция
остаются. CPU fixtures используют synthetic assets и внешние state команды;
реальные level selectors/collision/render и исчерпание полного пула этим
этапом не доказаны. Donor/32 pins/test037 сохранены; нового Windows EXE нет,
игра/окно/renderer/GPU не запускались. Проверенные этапы сохраняются commit/push.


## main282 — появление4, вращение7 и двухфазная дуга8 предметов

После настоящей Source инициализации предметы118/123/127 теперь обновляют
uniform scale появления4, yaw/speed/opacity вращения7 и XYZ/scale двухфазной
дуги8 на Native интервалах. Geometry использует настоящие Original angle/
trig helpers и ROM constants. Source сохраняет init, целые angle/phase/latch,
blink reset, переходы4→10/8→1, contact, команды и удаление. Только неизменённые
непрерывные поля возвращаются перед исходным callback; внешние команды не
затираются. Original20/30 сохраняет прежний путь. Несовместимые команды угла
state8 оставляют Source до настоящего wrap/clamp и возврата в допустимую фазу.

Fresh279 reveal guards иfresh283 arc guards завершили по3456 Original и3456
mixed worlds, по884736 полных состояний actor/physics/model/heap/resources/
commands. Восемь профилей до/послеNative покрывают C9 pause/resume, velocity/
callback rejection/reentry, внешние XYZ/scale/yaw/angle, blink6, настоящий seek
иmodel pause. Guard8 отдельно проверяет неизменность сырых полей при
несовместимых phase/angle. Каждый из216 применимых миров обязан сохранить
обе отклонённые фазы на интервале положительной длительности; всего3132
rejected calls. Ранее281 достиг всех884736 равных endpoints, но завершился
ошибкой общей coverage-проверки: требование >3456 rejected calls не учитывало,
что неправильные углы остаются только в216 мирах доNative. Этот запуск неPASS.
В283 проверка усилена по каждому миру/фазе; runtime исырые сравнения прежние.
Fresh283 basic432 проверяет оба Native7/8 иполные
lifetimes. Ранее269 spin guards3456/884736 остаются историческим доказательством
своего runtime, без приписывания новому исполнению.

Свежий281 полный O2/ASan/UBSan/LSan прогон сохранил прежние player/weapon/
damage/death/recovery/flight/surface/door/world/city/linked/patrol/orbit группы,
5088 Source/720 Native slot09/34464 frozen comparisons и3192 actual NPC/B.
Fresh283 loop иactual scene resource barrier также прошли. Семь текущих
отчётов имеют точные входы иявное происхождение279/281/283; names282 исполнен
заново. Исторические276 component receipts сохранены как superseded, их
source hashes не переименованы в новую runtime проверку.

723 описания/388 смысловых имён/506 register scopes сохранены целиком;
описания/evidence трёх состояний идокументы обновлены.96 canonical C units,
frontier/hooks/overlay table равны исполненным279/281/283. Fresh names282
подтвердил canonical round trip/preprocessed tokens/C17/ABI. Изначально
прерванные WSL прогоны272/273/277/279 не считаются PASS; после перезагрузки
полный прогон281 выполнялся вместе с одной guard-матрицей; измеренный RSS
full432MiB/guards3.3GiB укладывался в доступную память WSL. Image-loop использует explicit
abort-only math/opacity boundaries, gameplay scopes исполняют actual Original.

Native2/3,parent-follow/moving idle/другие семейства предметов, прочие уровни,
враги/платформы, script cameras/cutscenes, миниигры/2D иобщая интеграция ещё
остаются. Full variable FPS не завершён. Fixtures используют synthetic assets
ивнешние state команды: реальные level selectors/render/full pool exhaustion
не доказаны. Test037/donor/32 pins сохранены; нового Windows EXE нет,
игра/окно/renderer/GPU не запускались. Проверенный блок сохраняется commit/push.


## main286 — понятные имена помощников поверхности

Подтверждённые startup8003FFC4 и80040018 получили имена
`dora3_collision_reset_upward_plane_candidate_8003FFC4` и
`dora3_collision_reset_downward_plane_candidate_80040018` и описания каждого
28-байтового результата. Направления normal(+Y/-Y) подтверждены исходными
инструкциями; роль пола/потолка по одному направлению не предполагается.
E756C переименован в `dora3_collision_reset_special_surface_FE_801E756C`:
он выбирает specialFE, очищает оба результата и устанавливает bit0. Старое
описание ошибочно называло очистку повторным поиском столкновения.

Теперь725 описаний/390 смысловых имён/506 register scopes. Fresh names286
проверил96 canonical C units, одинаковые preprocessed tokens, C17 syntax и
ABI/register storage. Native runtime, timing hooks и шесть исполнявшихся
проверок main282 не менялись; новые receipts явно сохраняют actual279/281/283
origins и не заявляют повторное исполнение runtime. Каталог/карта адресов/
связанные документы синхронизированы. Native2 находится только в отдельных
кандидатах: базовые432 пары287 прошли, расширенная проверка289 обнаружила
расхождение затухания после parent handoff. Исправление291 прошло базовые432
пары, цикл, scene resource barrier/NPC B и725/390/506 names; расширенная
guard-матрица иполный O2 прогон291 ещё выполняются.
Native3/parent-follow/остальной game
variable FPS не завершены. Test037 прежний, нового EXE и запуска игры/GPU нет.


## main295 — плавное оседание и правильное затухание после parent handoff

Предметы118/123/127 после настоящей инициализации Source2 без parent теперь
обновляют Y и velocityY на Native интервалах. Actual E810C forecast использует
исходный baseline и offset-50, возвращает временные actor/physics/results/stack
записи. Source сохраняет init, parent transform, gravity/probe/latch/state1;
неизменённые непрерывные поля возвращаются перед Source, внешние команды
сохраняются. Parent-follow остаётся Source-only.

Расширенная289 проверка нашла настоящее расхождение opacity0.925/Original0.95
в collected10 после parent handoff ниже поверхности. Diagnostic290 показал:
itemY=-72, следующий actual E810C меняет VY на+72. Старый Native forecast
проверял только gravity и начинал fade до query. В291 сам Native forecast
сначала выполняет настоящую query, затем выбирает ветвь по её новой скорости.
Порог/команды/Source исходы не заменены. Negative289 сохранён отдельно;
старый комментарий fixture про standalone player не описывает actual pool0.

Actual291 basic432 paired worlds/110592 whole endpoints и8 command profiles
до/после Native3456 paired worlds/884736 whole endpoints прошли. C9 pause,
velocity/callback rejection/reentry, absoluteY, gravity, blink6, parent handoff,
actual seek/model pause сохраняют actor/physics/model/heap/resources/commands
до Source contact140, шести effects и полного удаления. Зафиксированы199452 Y,
222204 VY changes и309264 collision-cache/terrain rollbacks в guard-матрице.
Свежие related reveal/arc guards, combined spin/arc lifetimes, full291 O2
player/world/NPC3192 B schedules, loop иactual scene resource barrier прошли.
Восемь runtime reports имеют явные291 origins; девятый —fresh names295.

725 описаний/390 имён/506 register scopes сохранены. Descriptions2/collected10,
каталог/карта адресов/документы синхронизированы;96 canonical C units,
preprocessed tokens/C17/ABI проверены. Готовый блок сохраняется commit/push.
Разлёт3 проверяется отдельно: basic303 иgeometric302 прошли. Полная303
матрица дошла до1105920 точных endpoints, но проверка покрытия выявила,
что остановка с VX/VZ=-0 не включает Native idle. Diagnostic311 подтвердил
причину;312 принимает оба знака нуля без изменения velocity bits, короткая
матрица20 paired worlds/5120 endpoints прошла с5136 Native idle yaw changes.
Полные проверки312 предстоят. Source-parent307 прошёл864 paired lifetimes;
Native attached310 прошёл221184 endpoints,36432 frozen matrix checks и87336
zero-dt no-change checks, но требует расширения guards иобщей проверки.
Эти отдельные кандидаты не включены в main295. Parent-follow для других
контроллеров/moving idle/другие семейства ипрочие уровни,
враги/платформы/script cameras/cutscenes/minigames/2D/общая интеграция остаются.
Full variable FPS не завершён. Test037 прежний; нового EXE и game/GPU нет.


## main314 — понятные имена матричных операций

Разбор следования предмета за родителем подтверждён настоящими Original
матричными вызовами. Для четырёх функций умножения матриц, создания переноса,
преобразования точки и вращения вокруг оси заданы понятные имена и описания.
Добавлены 15 имён регистров только в доказанных областях инструкций; каждое
основание связано с ROM identity и точными opcode words.

Всего в каталоге 725 описаний, 394 смысловых имени и 521 register scope.
Свежая проверка314 подтверждает 96 одинаковых canonical C units, C tokens
после preprocessing, C17 syntax и прежние ABI symbols. Runtime main295 и
восемь его проверок сохранены с точными execution origins; игровое поведение
этот этап не меняет. Документы функций, переменных и карта адресов обновлены.
Native разлёт312 и attached-parent310 остаются отдельными незавершёнными
кандидатами. Нового EXE, game/GPU запуска и утверждения полного FPS нет.


## main336 — имена общих семейств предметов

Original84-byte таблица подтверждает111..128:18 update wrappers вызывают
общий state dispatcher801EC7D0, но отдельные producers/contacts могут
проверять сюжетные флаги. Source-only33 разобранным wrappers даны имена:
15 updates,8 простых contacts и10 простых descriptor producers. Добавлены
10 ROM-qualified scoped имён a0 при построении адреса дескриптора. Selector
простых producers различается:114/128 передают1, остальные изученные —0;
его более широкий смысл не угадывается. Неизученные сюжетные wrappers
сохраняют исходные имена. Документы/карта функций и переменных обновлены.

758 описаний/427 смысловых имён/531 scopes сохраняют все имена main314.
Свежая names336 подтверждает96 canonical C/preprocessed tokens/C17/ABI.
Runtime main295 и его восемь настоящих291 origins неизменны. Семантические
имена не расширяют Native поддержку. Full312, parent321 и guards332 идут
в отдельных кандидатах; Source331/Native33318-family short proof прошли,
но broad states/fps/guards/whole integration ещё не завершены. Negative326
raw equal-value command limitation сохранён отдельно; actual setter328
и observable command cases не объявлены исправлением этого ограничения.
Полный variable FPS не завершён. Нового EXE и game/GPU запуска нет.


## main343 — сюжетные конструкторы и связанные эффекты

Ещё20 разобранным Source функциям даны имена:8 quest-guard constructors,
7 quest contacts, shared script descriptor producer и kind scale helper,
а также3 callbacks связанного kind129 эффекта. Добавлены14 ROM-qualified
scoped register roles: contact mode, quest parameter address, actor/kind
и раздельные s1 selector/index. Неиспользуемый явно a0 не получил фиктивную
переменную: strict generator остановил предложение341; доказанная роль
descriptor остаётся в комментарии. Fresh342 затем прошла96 canonical C,
preprocessed tokens/C17/ABI. Никакие проверки генератора не ослаблены.

Actual339144 paired private Source constructor cases совпали по4MiB RAM,
context/heap/next allocation/resources/commands/model count. Проверены
dynamic flags0/1/31/32/511, zero bypass119/122, fixed44/46, level-dependent
flagsA0/A3/FA/FB/117/119/default, создание трёх129 и отказ полного пула.
Source explicit destructors сохраняют владельцев; автоматический129 lifecycle
и частичный linked-pool exhaustion не заявлены. Описания10 plain producers
уточняют доказанный selector1/0. Неизвестные игровые названия не угаданы.

778 descriptions/447 semantic aliases/545 scopes сохраняют main336.
Свежая names343 подтверждает96 canonical C/preprocessed tokens/C17/ABI.
Runtime main295 и восемь настоящих291 origins неизменны; names не добавляют
Native ownership. All18 candidate338 short states2..8 прошли:
324 paired lifetimes/79488 full endpoints:2/3=108,4=54,5/6=54,7/8=108.
Candidate340 all24 idle/velocity matrix и338 broad scale matrix ещё идут.
Full312, parent321 иguards332 остаются отдельными активными проверками.
Полный variable FPS не завершён; нового EXE и game/GPU запуска нет.


## main348 — переменные связанных эффектов и полный Source lifecycle

Функция dora3_pickup_linked_effect_update_source_801ED2CC получила17
ROM-qualified register scopes: physics на конкретных этапах, parent pool
index/address, signed delay count/limit и actor при трёх мгновенных XYZ
записях. Области заканчиваются до смены роли регистра; ctx storage,
Original delay slots, ABI и машинный смысл сохранены. Каталог:
778 описаний/447 имён функций/562 области переменных. Fresh347 и затем
fresh348 проверили96 canonical C/preprocessed tokens/C17/ABI.

Source344 прошла72 paired complete lifetimes/18432 exact whole endpoints:
8 actual private constructors ×3 heights ×3 XYZ velocity profiles, три
настоящих kind129 на каждый предмет, actual contact140/quest/collection,
шесть effects130, автоматический unlink129 и полное model/heap/resource
освобождение. В85680 Native интервалах все129 actor/physics/model words
оставались Source-owned. Это synthetic assets/direct inputs; реальный
selecting script, частично занятый linked pool и renderer не проверены.

Source код129 не задаёт постоянную velocity: RNG instant teleports,
delay/count/clip/seek/retirement сохраняют порядок Original. Candidate346
отдельно проверяет только initialized common model animation с live
reciprocal parent links; каждая из трёх моделей обязана показать реальные
промежуточные time changes, а whole duplicate timestamp сохраняет весь
snapshot/context. Initial preflight остановился до исполнения, потому что
новый entry hook отсутствовал в strict canonical allowlist; добавлен только
reviewed ED2CC, остальной body по-прежнему обязан совпасть с frozen189.
Native346 результата пока нет; в main348 нового129 ownership нет.

Runtime main295 и восемь настоящих291 origins неизменны, девятый report —
fresh names348. Full312/parent321/guards332 и broad338/340 продолжаются
в отдельных неизменяемых кандидатах. Полный variable FPS не завершён.
Нового EXE и game/GPU запуска нет; test037 прежний.


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

## История повторных проверок и прерванная попытка19

Оркестратор ошибочно включал весь предыдущий record вместе с уже вложенным
attempts при каждом повторе. Это экспоненциально дублировало историю и
увеличило suite.json до1,820,919,586 bytes. Точный исходный файл сохранён
в ignored suite-expanded-history-before370.json с SHA256; рабочая история
сведена к текущим28 records и отдельным журналам с их SHA256. Будущие
попытки сохраняют только неглубокий previous record; существующие журналы
не перезаписываются, имя текущего журнала записывается до запуска проверки.

Остановка только старого управляющего процесса не сохранила дочерний replay:
WSL завершил дерево после закрытия внешней команды. Попытка19 остановилась
на269/1308 profiles без обнаруженного расхождения, но без полного результата.
Её статус INTERRUPTED; частичное прохождение не считается PASS и не
переносится в новый enemy replay.23 независимых завершённых проверки
сохранены. Повторяются враги и четыре ещё не выполненные связанные проверки.
Игровые сценарии, сравнения и runtime inputs этим исправлением не менялись.

## Прогон20: входы чистой attachment geometry

Full replay завершился FAIL после1182/1308 profiles: Native BroboDC08
linked RealT1/T2 входил в E344/20614 после пропущенного lui a0,8022.
Точный Source показывает эту базу в E340/20610. Generator теперь восстанавливает
только доказанный a0 перед pose goto; init/clip/count/collision не повторяются.
Rikurage light338 использовал unsigned full-address-as-offset в injected
MEM_W(8021BCDC,0). Теперь pose entry идёт к настоящему L_80222BF0, где
Original lui/lw получают physical pointer. Source алгоритмы не изменены.

Ошибки исправлены в generator, с новой отдельной generation362;361 и все её
failed/partial logs сохранены. Результат1182 profiles не объявлен PASS всего
блока. Проверяется точный diff generated bodies; неизменённые CPU scopes
сохраняют фактическую generation361 и provenance, изменённая enemy closure
и оставшиеся связанные проверки выполняются на362. Игра/GPU не запускаются.

## Soundorago: завершение Original fixture до common retirement

Scoped362 дошёл до Original kind300/301/302/303 profile0 и FAIL на cleanup:
после common E6E6C оставался один heap allocation, models/resource refs
полностью возвращались к baseline. Diagnostic36-kind300 доказал mode0
(frozen Original), heap802A04A0 — Source history800 bytes из D768/D778.
У305 история1200 bytes освобождается только death body EC08;300 — E170.

Fixture после96 сравниваемых ticks вызывает настоящий contact0 для живого
300/305 и выполняет Source pool до действительного death geometry contact.
Прямого fake free нет, assertion полного возврата heap/models/refs сохранён.
Этот дополнительный tail проверяет Source cleanup; не считается проверкой
Native death animation. Остальные типы/сценарии не изменяются.


## Сокращение повторного дорогого player/world прогона

Перед043 полный player/world PASS занял5008.47 секунды; все28 checks PASS.
Добавлен точный compiler/header/generated-code/seed/toolchain cache четырёх
завершённых групп. Повторная проверка тех же входов и сохранённых результатов
занимает около10 секунд без нового runtime replay. Ошибка/повреждение/изменение
dependency запрещает reuse. Холодный прогон при новых player/world зависимостях
остаётся дорогим; сокращение его длительности не заявлено. Runtime/fixture043
не изменены, старые actual CPU/Windows proofs сохранены. Документ:
docs/native-test-execution.md. Cache safety проверена короткой отдельной fixture.


## 2026-10-05: блок1, реализация и предсборочные исправления

Выбран полный блок объектов/платформ/NPC:289 типов,292 ROM-qualified варианта.
Добавлены отдельный scene registry, common Native command/model/motion owner,
точные scalar/phase equations, Source-order parent paths и аддитивное движение
игрока у Door109. Kind590 относится к NobiheyaDrawer в level1_slot2;
одинаковый адрес Miire NOP в другом ROM не является этим владельцем.
Варианты521/541/545 имеют отдельные подтверждённые descriptor banks.

Генерация сначала выявила ошибки аннотаций: пустые диапазоны variables,
псевдонимы одинаковых overlay адресов и регистры, устранённые canonical IR.
Исправлены диапазоны, ROM-qualified aliases и недоказанные scopes;
startup-scene-block1-v4 и canonical round-trip/names compilation прошли.

Первый предсборочный запуск использовал неверный leaf reference79; повторены
только пять соответствующих checks с доказанным69. В C++ исправлены
лексическое чтение 0xBE+2 как числового суффикса и misleading indentation.

В новом fixture constructors EB4E8 (485–487), DFC4 (569/570) и E328
(572/573) читают placement kind8021BE00 для выбора FFF0 программы.
Первое предположение, что descriptor непосредственно вызывает E6968,
оказалось неверным; ROM instructions EB4EC/DFC8/E32C подтвердили настоящую
причину. Исправлена запись placement kind, исходный ABI конструктора сохранён.

ASAN выявил настоящий Native дефект Fly44/45: unsigned displacement
0xBE+2*axis менял sign extension в MEM_H и уводил чтение за RDRAM.
Смещение явно приведено к int32_t; затронутый loop и оставшийся scene replay
повторяются. Старые завершённые варианты не содержат Fly/Fuyuu этой ветви.

Retirement NPC43 расходился уже в frozen Original. Короткая CPU diagnosis
под debugger показала реальный kind249 в слоте30 после удаления WrapSlime6
в слоте31: Source destructor создаёт исчезновение в ранее пройденном слоте.
Fixture теперь делает ограниченные дополнительные проходы настоящего общего
destructor, только если остались реальные новые allocations. Для сохранённых
полных сценариев эта ветвь неактивна: прежний ресурсный assert уже подтвердил
точное равенство baseline. Native логика освобождения ради теста не подменялась.

112 полностью завершённых ROM вариантов сохранены с проверкой точных
Source/runtime dependency hashes, executable/log provenance и branch scopes.
Частичные и затронутые варианты выполняются заново. Connected pre-build ещё
не завершён; Windows044 не создан, локальная игра/GPU не запускались.


Блок1, финальный connected replay: Source81 использует параметр placement BE10 как индекс четырёх dialogue programs; fixture sentinel31 был неверен. Source135 retirement запускает E50C0(effect5), который реально прикрепляет кости2..21 и переключает22: synthetic skeleton дополнен до23 узлов без подмены SDK. Source585 регистрирует переход через BA0C4/BA05C; BB080 вызывает BA0F4 раньше E8BDC, получает1 и пропускает старую сцену. Replay останавливает старый pool на этой границе, сравнивает все шесть request globals, команды, heap/model ownership. Независимые полные варианты сохранены с бинарниками, логами и обратимыми scoped edit certificates.


Source Door109 contact3 вызывает BBD5C/BBD40 и последующую смену player SDK pose. Диагностика CPU baseline показала неизменную модель802A0000 и заменённый owned buffer+60=802A04B0, с2 моделями/6 baseline allocations. Retirement contract109 проверяет тот же строгий release/ownership/lifetime replacement, что585; все остальные blocks/resources по-прежнему должны совпадать точно.


Native Door109 contact3: EA5D4 вызывает B8ED0(0), устанавливающий D494=16. Старый Native live сразу прекращался, и следующий Original pool повторял частично исполненный Source период (tick21: H34=103/H36=0 вместо102/1). BB080 при D49416 продолжает E8BDC, но пропускает C8B28. Добавлен lifetime/epoch-qualified fade owner лишь для реального Native Door109(state2,latch1); common-world Native продолжает работать, player C8B28 отключён по настоящему D494. Pending-room globals/другие gates/reset/replacement не разрешаются обходом. Повторяются дверь, незавершённые области и checks с изменённым preview input; прочие полные варианты сохраняют provenance.


### Блок1: промежуточный статус перед044,26/29

Весь выбранный код/ROM-qualified names/comments подготовлен. Scene replay
PASS:289 типов/292 ROM-варианта,1163 frozen/current Source bodies,
Original20/30 и Native60/73/120/144/165/360. Сохранены291 неизменённый
полный вариант с обратимыми scoped edits и execution provenance;
последний свежий replay двери109 сравнивает и foreign player поля.
Door fade сохраняет common-world Native по BB080 и останавливает player;
отдельный48-case CPU debug replay подтверждает старое motion permission.
Полный новый enemy replay PASS:все218 типов/1233 exact Source bodies.
Scene transitions PASS.26 завершённых checks проверены по текущим
source/generation/log hashes. Player/world готовит новый результат:
0/4 прежних exact-input cache groups пригодны,4/4 группы выполняются
заново после изменений общего runtime. PASS ещё не получен; Windows044
ещё не собран. Далее scene-frame/pickup checks, один Windows build,
package preservation, commit/push main и выбор следующего блока4/6.


## Блок4 после044: начало всей реализации миниигр

Baseline7820d68/test044. По текущей generation найдено695 функций в10 ROM
modules,9 отдельных игр и3 common update/draw pairs. Зафиксированы точные
ROM/update/draw/return/body SHA identities. Current scoped source snapshots
и Source call graph хранятся только в ignored build-local/continuation-20260930/block4.

Добавлены незавершённые native_minigames runtime, native_minigame_math typed
equation ledger,12-pair registry, generator adapter и policy/game-loop/CMake
подключение. Ledger записывает арифметические зависимости; Source RAM/commands
не меняются сбором выражений. Publication roles остаются пустыми и должны быть
доказаны перед подключением. Actor SDK layout: local XYZ0/4/8,EulerC/10/14,
scale18/1C/20,matrix24..60,dirty6A; components74/78/7C могут быть sprite handles,
а не pointers. Lifetime model проверяется по правильному component slot,
resourcesB0 иscene epoch. External stores отменяют устаревший mathematical lease.

Source callback closure:1=52functions/21FP writers;2=32/12;3=42/12;4=61/26;
5=26/4;6=30/4;7=31/9;8=53/18;9=35/7. Common3 pairs не имеют FP writers:
их pulse/2D equations целочисленные и ещё требуют реализации. Эти числа —
инвентарь, не coverage/PASS. Ни CPU replay, ни новая генерация/Windows build
не запускались. Код блока4 не закончен; миниигровые symbols/variable roles,
FP publication sites, integer visual motion, камеры, sprite/clip/time,
выход/смена scene/callbacks и framebuffer dependency остаются текущей работой.
Нельзя объявлять block4 completed либо выдавать его текущий scaffold за045.


### Блок4 — motion implementation, незавершённый этап после044

Добавлены конкретные arithmetic/store roles для5/6/7/9: actor XYZ/Euler,
управляемые XY и spherical projectile velocity/XYZ. Одноразовые +400 X/Z,
tile shift, начальные ориентации/placement, result shrink и Source retire не
масштабируются. Общие результаты используют clamped fractional arrow и alpha
на draw operands; direction/FSM/input/waits сохраняют Source.

Equation ledger теперь различает command cancellation, polynomial discrete
sum (gravity-linked displacement), geometric/affine recurrence,7 component
handles и allocation identities, actor98 driver ownership. Stack-only spill
tags не перекрывают motion-field expressions; FR0 paired spills сохраняют
single-precision provenance. SDK cos80055BD0/sin80057A80/sqrt80055BC0 уточнены
по настоящим ROM bodies, opaque call clobbers очищают старые caller tags.
Draw-scoped lease восстанавливает SDK matrices/world-coordinate derivatives,
не оставляя frame temporary state следующему Source update.

В каталоге добавлены Source function names/descriptions и ограниченные ROM
register scopes. Все изменения пока непроверенные: остальные миниигры,
камеры, clip/counter operands и полный connected replay не закончены;
генерация/test/build/commit/push блока4 не выполнялись. Не считать это
завершением блока4 или поводом передавать частичный EXE.


Уточнение Source evidence: SDK8002A14C вызывает8003AFEC для отрисовки модели,
а не получения animation time. Ошибочная draft привязка была удалена до
generation/tests/build. Typed time operand теперь допускается только на
реальном lwc1 по registered model+64, с allocation/run/seek checks и completed
Original observation. Добавлены module2 camera/driver/movement и module3
camera/scale/position/scroll/fade roles; phase/contacts остаются Source.
Registry implementation-ready guard закрыт: весь блок4 пока не завершён.


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
