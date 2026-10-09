# Остаточная производительность после080

Пользователь: «о улучшение в2 раза но до360 далеко». Подтверждает практический
эффект memoization, но не устанавливает новый предел и оставшийся bottleneck.
Предыдущий079 профиль относится к коду до оптимизации и не позволяет распределить
нынешний расход.081 observation-only сохраняет алгоритм080 и игровую логику.

Добавлены exact cache hit/miss/bypass counters и1/64 samples ObjectTimeline,
ObjectClockRegistration,ModelRatePlan,EnemyCallback,EnemyPoolFinalize,
ModelClipBoundaries,ModelFrameScope begin/end (последние вместе).
Это выбранные Native области, не полный профиль Source-функций.
В мелких samples удалены системные CPU/cycle запросы, которые раньше попадали
в wall duration и искажали особенно короткие triangle/vertex оценки.
Coarse begin также начинает wall после CPU query; coarse CPU остаётся грубым.
Schema3 отмечает thread_cpu_measured=false у sampled stages.
Глобальные счётчики и таймеры всё ещё создают observer overhead. Nested estimates
и параллельные threads нельзя складывать; 1/64 scaling является оценкой.

Нужен пользовательский запуск20–30с в прежней тяжёлой сцене с теми же настройками.
Агент только читает исходники/метаданные и компилирует; game/tests/CPU checks/GPU
не запускаются. Итоговый следующий performance fix должен быть чистым.

081 собрана: MSVC RelWithDebInfo, 33.797с;20 build steps; GRAPHICS_PROFILE ON, DIAGNOSTICS/Others OFF.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-081.exe
SHA256 bef7b3c5e88170b06f0945a6ee16eb767b14cbb80a6f27df2d83a3f83ab39d75
Все255 прежних файлов вне logs сохранены; sharedDLL hashes совпали.
Компиляция/упаковка exit0; game/tests не запускались; commit/push отсутствуют.
