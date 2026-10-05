# Thunder283–286: враги, генератор и разряд

Внесены полные Source closure четырёх типов: выбор направления, patrol/chase,
атака, clip commands, связанные births/unlinks, contact/death/retirement.
Native продолжает реальные bounded turns, direction accumulator, положительный
полуоборот180, ускоряющийся death spin и shrink с исходными scale copies.
У 284 rate34 увеличивается перед поворотом; ускорение8 сохраняет настоящий знак.

285/286 наследуют только yaw родителя. Proven geometry-only boundaries повторяют
это в промежуточных кадрах без entry, counters, orphan commands и births. Parent
slot и lifetime, child0 и его lifetime входят в command identity. Позиционное
следование, которого нет в Source, не добавляется.

22 функции получают semantic names/comments/scoped variables через каталог.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
