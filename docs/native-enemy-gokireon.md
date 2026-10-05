# Gokireon281 / Gokireon282

Внесены полные Source closure обоих variants с jump-table FSM, contact/recoil,
patrol, conceal/charge/animation sequences, flight/dive и death/retirement.
17 функций переименованы через каталог, добавлены описания и scoped variables.
Native equations продолжают настоящие heading/angle accumulation, speed,
opacity с cap, scaleX shrink, death Y+16/visualyaw+80 и их literal heading copies.

281 fade-in публикует opacity после arithmetic, conceal публикует предыдущую
opacity перед decrement. Учтены оба порядка без лишнего Source callback.
282 vertical flight/recoil impulses остаются командами, после которых Native
продолжает настоящую constant drift. Late walls/selected floor/caps/angle snaps
сохраняют исходные setters, RNG и все фазовые counters остаются Source.

Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
