# После072: Native full-pool subdivision; диагностический073

Пользователь:5x около30FPS; требуется сначала стабильный60, затем больший FPS.
Не объяснять эту просадку настройкой лимита. Последние профили072 сохранены
в build-local/profile-test073-build, агрегаты: research/test072-performance-analysis.json.
Manual запуск1791458554488 на5x:32.2FPS,107 common actors,10498 full pool
calls за20.34с (примерно516/сек), timeline608мс/сек; rate/dispatch/identity
расходы уменьшились после072. В Original запуске1791458247326 на5x132 actors,
304 common calls за15.18с (примерно20/сек), world gameplay около7мс/update.
Original output20FPS — его исходный режим, не новый hard cap и не performance proof.

## Механизм, требующий конкретизации

SimulationTimeline::advance_while дробит общий elapsed на Source tick, input
samples и ограничения Native preview planner. Ограничение одной модели/личного
периода заставляет continuous callback снова выполнить весь common pool,
игрока, projectiles и камеру. Рост born/local actor phases может умножать работу.
Нельзя убрать границы или выровнять их наугад: docs/native-simulation.md
фиксирует настоящие зависимости birth, short-angle, clip cue и child ordering.
072 не записывал тип ограничителя и распределение размеров slices, поэтому
нельзя честно назвать конкретный boundary причиной всех516 проходов/сек.

##073: один точный замер

timeline.limit_reasons: массив из5 counters —0 обычный Source tick/оставшийся
интервал,1 model clip,2 object local period,3 projectile period,4 player cue.
Счётчик указывает группу, последней уменьшившую окончательный limit; равные
ограничения сохраняют предыдущую группу. Уравнения/порядок min/limit неизменны.
timeline.slices: реальное число continuous callbacks; input_segments/input_edges
отдельно отражают деление входными samples. elapsed_ms/max_gap_ms показывают
прошедшее логическое время и максимальный catch-up gap.
slice_histogram: число slices с верхними границами времени в ns:
[1,1000,10000,100000,1000000,5000000,10000000,без верхней границы].
Рational units переводятся в ns делением на60; дробные наносекунды попадают
в первый bucket. Это позволяет отделить обычные периодические события от
разрастания почти нулевых шагов.

У основных stages Native update/frame, object timeline, Source gameplay и
RT64 parser добавлены cpu_calls/cpu_ms по thread CPU clock: Windows GetThreadTimes,
Linux CLOCK_THREAD_CPUTIME_ID. cpu_calls0 означает отсутствие этого измерения.
cpu_ms исключает ожидания/дескедулирование; учитывать гранулярность системных
counters и overhead, сравнивать несколько секунд. Вложенные cpu_ms тоже не
складывать. Это запись во время пользовательского запуска, не CPU check агента.

Доработка предыдущего immutable-index изменения: native_enemy_kind и
native_scene_object_kind теперь используют те же constexpr индексы вместо
повторного std::any_of. Это те же множества kind, без mutable ownership cache.
Остальные изменения073 — наблюдение. При удалении вставленных observer строк
Native preview body побайтно совпадает с072, включая все уравнения/границы.
Source review: research/profile073-source-review.json. Генерация CPU прежняя;
AI/contacts/birth/clip/time/draw limits не изменены. FPS073 пока не проверен.

Для пользовательского замера достаточно того же места:5x15–20 секунд,
пауза5 секунд, затем закрыть игру. Агент читает автоматический performance.jsonl.
Не запускать игру/тесты/CPU checks/renderer/GPU агентом. Git после проверки.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 27.0 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-073.exe.
SHA-256: `10a1868e14b15a88a3dd03ff6bf9e318ddd5c684463738aa0cd5bde10a7a6b2d`.
Все 231 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.
