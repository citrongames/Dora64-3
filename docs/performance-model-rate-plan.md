# Падение FPS на повышенной дальности: профиль069 и изменение070

## Измерения

Пользовательский запуск069: logs/1791455799537/state/performance.jsonl,
59 samples, scope_errors=0. В scene165 (лес фей) на2x —60FPS.
На4.1x последний sample охватывает25.9318 секунд и один output frame:
object_timeline25.885с, self16.7212с; common_actors6746 вызовов/7.42128с.
world_gameplay7 вызовов/20.0862с включает вложенные Native обновления;
эти интервалы нельзя складывать. GPU0.946944мс, RT64 parser12.5096мс,
world draw2.253мс. Контакты7 вызовов/30.9052мс. До игровой паузы запуск
не дошёл; port_menu — меню порта, не игровая пауза.
Полные агрегаты и SHA-256 исходного профиля: research/test069-performance-analysis.json.

## Установленный участок и механизм

Замер локализовал провал внутри Native gameplay timeline, преимущественно
вне Source common/player/projectile callbacks. Статический разбор выявил
вложенный поиск: clip_boundary_limit проходит все зарегистрированные модели,
а planned_model_rate для каждой модели заново проходит common pool30..599
и выполняет owns_common_object до сравнения указателя модели. Многократно
повторяются полные проверки identity, type/overlay/table/lifetime всех
предыдущих живых актёров, а не только владельца текущей модели.
Локальные периоды рождения, границы клипов и входные samples делят Native
интервал на несколько slices; тот же поиск повторяется в каждом slice.
Чем медленнее обновление, тем больше wall-time gap следующего update и
повторных проходов при catch-up. Профиль показывает это разрастание.

069 не имел отдельного таймера поиска rates, поэтому точный процент его
затрат и причины всех subdivisions не измерены. В070 добавлены отдельные
model_rate_plan, model_clip_boundaries и object_clock_registration. Улучшение
FPS после изменения070 пока не подтверждено пользовательским запуском.

## Изменение070

На одной read-only boundary формируется таблица model→rate за один проход
по игрокам/common/projectiles. clip_boundary_limit получает прежние rates
через поиск в этой таблице, без повторного прохода570 слотов для каждой модели.
Таблица создаётся заново после каждого фактического slice, не хранится между
изменениями RAM, lifetime, state, overlay, pause или command. Ownership
predicates не изменены. Первый совпавший owner сохраняет приоритет:
primary player, mirrored player, common по возрастанию слота, projectiles.
C9 exclusion, rate0→3, primary state46→1 и все прочие rates сохранены.
Разбор lookup и clip callback подтверждает, что между построением таблицы
и её чтением нет продвижения симуляции или guest RAM writes.

AI, contacts, callbacks, clipping/animation edges, birth phases, full elapsed
time и catch-up policy не менялись. Original не использует этот Native
boundary plan; Native/Manual используют один и тот же исправленный lookup.
Генерация CPU та же, что069; игровые инструкции/ROM hooks не изменялись.
Пулы/дальность, соседний Dora64, sources/ и pinned dependencies не изменены.
Проверка агента — статический разбор и компиляция/упаковка; запусков нет.
Git остаётся незакоммиченным до пользовательской проверки и команды.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 26.891 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-070.exe.
SHA-256: `d35c6a88741f2176db1af0729b240998fbbf05c00793e8af4bef9f9a20fcbad8`.
Все 222 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.
