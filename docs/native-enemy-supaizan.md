# Supaizan312–315 в составе блока2

В рабочую реализацию включены оба паука, паутина и снаряд, их полный Source
FSM, constructors/contacts и lifecycle. Четыре ROM-qualified owners допускают
Native model animation. Добавлены рост scale, вращение, торможение, fade и
наведение снаряда; точные Source cue39 создают выстрелы в исходном порядке.

Доворот тела312 захватывает старый roll до записи промежуточной разницы в
physical60/58. Его копия в scratch field получает ту же equation. Scale copies
X→Z/Y используют фактический X operand, включая anisotropic initial values.
Паутинный opacity публикуется до уменьшения58, с исходным first bind.
Slot80/lifetime паутины входит в command identity родителя. Связанный projectile
летает самостоятельно: связь не подменена следованием за позой родителя.

Collision radius, floor/bounds setters, круговой snap radius240, Initial short
quantization, target queries, wait/RNG и births остаются Source commands.
25 функций получили смысловые имена и scoped pointer descriptions.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
