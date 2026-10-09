# Профиль070: оставшиеся просадки на4x/5x; уточняющий071

Запуск070: logs/1791456699892/state/performance.jsonl,
SHA-256 c32760ad7ed7f56d6ca26e689cc07c70c49defc9cc8bd7ff925259f7ce478d96.
74 samples, scope_errors=0. Пользователь подтверждает улучшение3x;
профиль показывает там58–60FPS. На4x33–50FPS, активная часть5x4.8–25FPS.

На5x временной интервал примерно1.0–1.17с содержит851–1029мс object timeline;
common_actors self489–632мс, model_rate_plan40–54мс, clip boundaries2–3мс,
object clock registration43–57мс. Предыдущий nested model-rate поиск больше
не доминирует. После прекращения pool/contacts обработки FPS возвращается60
при том же5x и123 живых common actors. Диагностическое world_pause=0 не
различает все игровые причины остановки логики; нельзя считать последний
отрезок доказанным изменением конкретного pause flag.
Полные samples: research/test070-performance-analysis.json.

## Зачем071

Common pool self включает Original tail и Native preview/axis/ownership/
overlay checks, которые070 не разделяет. Нельзя называть конкретную функцию
внутри этого участка измеренной причиной без отдельного таймера.071 добавляет
только наблюдение; новый предполагаемый FPS fix или упрощение логики не внесены.

Новые sampled stages: preview hook, enemy axis, common ownership, enemy
identity, native model clock (примерно1/16), overlay lookup (1/64), gameplay
eligibility (1/32). Enemy pool finalization измеряется полностью. Hash от
per-thread/per-tag порядкового номера выбирает samples независимо от guest RNG,
без привязки к одному и тому же pool slot. У каждой стадии есть calls (число
замеренных samples), observed_calls (все вызовы), sampled (true/false).
Оценка общей inclusive длительности sampled stage:
inclusive_ms*observed_calls/calls, считать по нескольким seconds; max/histogram
относятся только к реально замеренной выборке. Вложенные stages/самостоятельные
оценки не суммировать, self исключает только фактически измеренные дочерние
scopes. Timing overhead остаётся ограничением любого instrumented build.

В071 после удаления вставленных Scope строк четыре изменённых gameplay
source файла побайтно совпадают с070: research/profile071-source-review.json.
CPU generation и ROM instruction/hooks прежние.070 rate table сохранена.
Нет изменений времени, AI, contacts, clips, birth phases, дальности или лимитов.
Нет запусков игры/CPU checks/тестов/renderer/GPU агентом. Git после проверки.

## Пользовательский замер

В лесу фей выставить4x на15 секунд, затем5x на15 секунд и игровую паузу
на5 секунд. Лог пишется автоматически. Закрыть игру и сообщить о завершении;
агент читает logs/<запуск>/state/performance.jsonl с локального диска.
По результату назвать конкретный измеренный участок и внести целевое изменение.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 27.625 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-071.exe.
SHA-256: `1e92e09247d4ebeb4e70a46fb802ba4185689c041ee752ca7d509c6efb49c511`.
Все 225 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
Эффект изменения на FPS требует пользовательского профиля.
Commit/push не выполнялись.
