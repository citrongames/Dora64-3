# Fisher330–331

Внесены полные closure обеих рыб: wander/patrol, chase, волновая атака,
ограничение глубины, contact и death. Сохранён порядок previous turn correction
перед расчётом следующей, pitch fold и floor/depth setters. Native продолжает
реальные yaw/pitch, visual copies, death roll и независимые wave operands.

Общий projection дополнен actual heading waves и накопленной sine turn
recurrence, а pitch wave учитывает sinusoidal scalar через harmonic sidebands.
Диапазон Bessel math расширен до pi по разобранным yaw90/pitch60 operands.
12 функций получают semantic names/comments/scopes. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
