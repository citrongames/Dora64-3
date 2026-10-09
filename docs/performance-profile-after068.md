# Профилировочная сборка069 после068

Пользователь:068 пока без замеченных вылетов; Fairy Forest и другие action
levels при3x..5x падают до5–10FPS и ниже,1x/2x нормально; города на5x стабильны,
пауза возвращает FPS. Manual60, RTX2060 из существующего пользовательского лога.
Причина НЕ установлена. Неподтверждённые оптимизации отменены; игровая логика,
дальность, лимиты и instruction/hooks manifest совпадают с068.

## Что записывает069

logs/<запуск>/state/performance.jsonl, одна строка примерно в секунду.
Current distance/scene/pause/FPS mode, число кадров, живых common actors и
допущенных к контактам, RT64 vertices/faces/matrices. Вложенные CPU wall timings:
Native update/frame, Original world update/gameplay, actor manager, common FSM,
contacts/player/projectiles, world draw/geometry/model draw, SDK animation
advance/evaluate, Native pose sample/restore, texture bind/clones, main DL и submit.
Trace измеряется выборочно1/64, число всех его вызовов записывается отдельно.
RT64: DL parser/VI update, renderer CPU с отдельными GPU/upload waits,
существующие GPU timestamps, workload total, present swap/workload wait.
Среднее получается inclusive_ms/calls; max_ms и histogram показывают пики.
Вложенные inclusive timings и параллельные CPU/GPU стадии нельзя суммировать;
self_ms у вложенных scopes исключает измеренные дочерние интервалы.
CPU wall time включает preemption/ожидания; отдельные wait stages помогают
разделить это. Trace sample — оценка по выборке, а не полный замер каждого вызова.

## Сохранение поведения

Генератор --profile автоматически оборачивает14 подтверждённых ROM функций;
весь прежний body и early returns остаются внутри, без изменений инструкций.
RECOMP_FUNC noinline/rounding attributes сохраняются, ABI/overlay function
pointers ведут к прежним именам. Статически сравнены тела с068 побайтно.
Profiler не пишет в guest RAM, не меняет Source/Native cadence, дистанцию,
AI/contacts, очереди, ожидания или renderer policy. RT64 наблюдается только
через build-local адаптер, pinned dependencies и соседний Dora64 не изменяются.
Профиль включается флагом generation receipt; обычная генерация без --profile
не добавляет wrappers и не включает запись. Поведение/FPS069 пока не проверены.

## Пользовательский замер

В том же месте леса фей: постоять15–20 секунд на1x, затем3x и5x по15–20 секунд,
открыть игровую паузу на10 секунд и снять её. После этого закрыть игру обычным
способом. Настройки и сейвы сохраняются; сравнение ведётся по дистанции/паузе
в профиле. Агент игру/renderer/GPU, тесты или CPU checks не запускает.
После лога сначала назвать измеренную причину; только затем целевой fix.
Commit/push после пользовательской проверки и отдельной команды.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 55.531 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-069.exe.
SHA-256: `b929d6da4e129935b54515d534ad6677dad9d945bdeb9aa7d25ce6df198df989`.
Все 219 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Причина падения FPS ещё не установлена: требуется пользовательский профиль.
Commit/push не выполнялись.
