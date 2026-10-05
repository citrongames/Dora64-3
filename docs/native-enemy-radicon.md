# RadiconRobo364 / RadiconCar365

Перенесены оба настоящих Source closure: ожидание/видимость, запуск, движение,
восстановление после стен/препятствий, contact и death/restart. Native math
использует реальные scalar/heading/pitch stores и фактические XYZ/XZ operands
до более поздних Source commands. Реакции на стены, cap, floor setters,
restart births, их отказы и state transitions сохраняют Source порядок.

Robo использует прошлый recovery rate5C до обновления этого коэффициента;
он не подменён новым target delta. Нормализация pitch продолжает ту же
bounded recurrence, а visual heading/pitch копируют только актуальный input.
Car по-прежнему использует отдельные XZ/floor+12 и bounce правила.

14 функций получили смысловые имена, описания и scoped variables.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
