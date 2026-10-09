# Профиль071: диспетчер Native hooks и поиск definitions;072

Пользователь прошёл1x/2x/3x/4x/5x и завершил игровой паузой.
Исходный профиль: logs/1791457405937/state/performance.jsonl.
SHA-256: 5019bf2a7834a82facf51a34d4fbf96fefea3bb4b50e95a2e32be9fa912002f3.
75 samples, scope_errors=0. Агрегаты: research/test071-performance-analysis.json.

## Доказанные измерением участки

Активная часть5x: 17.116670с, 1.402 output FPS,
до130 common actors. Preview dispatcher вызван10847908 раз;
самостоятельное время по hashed выборке примерно497.52мс/сек.
Проверка enemy identity вызвана14537837 раз и занимает по выборке
около255.65мс/сек inclusive. Gameplay eligibility
вызвана36701328 раз.
Эти вложенные оценки нельзя складывать. В частности, dispatcher inclusive
содержит рекурсивное Native обновление в Source BB080; он может превысить
wall time и не является процентом загрузки CPU. После прекращения pool/contacts
обновлений та же геометрия на5x выводится60FPS. GPU не объясняет провал.

## Конкретные ошибки в служебном коде порта

Original801E8BDC проходит весь common pool. В продолжении801E8E7C (инструкция
3C028022, уже квалифицированная в native-timing-rules/native_timing_patches)
порт вызывает единый preview dispatcher даже для неактивных/paused слотов.
Тот проходит десятки несвязанных site groups. Четыре guards дополнительно
вызывают live(ram) до проверки site: special city commands, generic axes,
floating dialogue, orbit turn. В конце остаётся ещё общая eligibility проверка.
На множествах Native substeps эта работа повторяется для570 слотов каждого
полного прохода. Накопление wall-time gap вызывает новые проходы catch-up.
Игра не становится настолько сложной: растёт лишняя работа адаптера.

Вторая дорогая часть: identity вызывает definition(kind), который линейно
просматривает218 постоянных enemy records и сценовые records. На миллионах
повторных queries заново ищется один и тот же immutable kind; изменяемые
overlay/callback/lifetime проверки должны оставаться отдельно актуальными.

##072

E8E7C сразу направляется в два прежних cleanup/publication блока в прежнем
порядке; они перенесены целиком. Они всё ещё выполняются для каждого слота,
включая пустые, paused, освобождённые и переиспользованные. Финальный live
observation сохранён для door memo. Остальные site groups не исполняются
для этого события, поскольку их predicates для E8E7C всегда false.
В четырёх guards сначала проверяется принадлежность события группе; для
совпавшего site порядок live/active и полный handler body не изменён.

Definition lookup теперь использует constexpr индекс, созданный из тех же
immutable records. Первый enemy record сохраняет приоритет; scene candidate
chain сохраняет исходный порядок и все возможные duplicate kinds. Live
overlay и callback/contact/constructor checks остались в прежнем виде.
Mutable actor/epoch/ownership/overlay решения не кешируются.

Статический source review и fingerprints: research/test072-dispatch-source-review.json.
AI/FSM, контакты, cleanup, clip/birth boundaries, elapsed time/catch-up,
дальность/пулы не сокращены. Original и Native/Manual получают те же решения.
Новых ROM адресов/инструкций/хуков нет, генерация CPU прежняя. Sources/, соседний
Dora64 и pinned dependencies не изменены. Компиляция не подтверждает FPS.
Profiler сохранён для сравнения; следующая пользовательская проверка4x/5x
и пауза должна показать изменение dispatcher/identity costs и FPS.
Игра/renderer/GPU/тесты/CPU checks агентом не запускаются. Git после проверки.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 18.547 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-072.exe.
SHA-256: `e481d5549ac9468966b302c266b4fed6e6c3d398fe9f8ef7338237170849f96f`.
Все 228 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.
