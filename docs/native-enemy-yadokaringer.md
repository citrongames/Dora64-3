# Yadokaringer332–335

Внесены полные closure обеих variants и пузырей: idle/recoil/chase, связанные
charge/release, прыжок/hover/landing, contact, shrink/fade и retirement.
Native продолжает реальные scalar ramps, turns, pitch waves/pulses/scale copies,
death spin/roll/radius. Short3A остаётся Source counter; captured kick используется
только зависимым visual roll. Attached333 повторяет только geometry с lifetime guard.

335 складывает две отдельные Source projections; runtime сохраняет оба набора
фактически использованных operands и отдельно продолжает их дискретные суммы.
Quadratic harmonic helper использует Gaussian power series/Bernoulli corrections,
при нулевом angular acceleration сохраняет exact harmonic expression. Source
endpoint остаётся actual AF float bits. Эта math требует финальной проверки блока.

23 функции получают semantic names/comments/scopes. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
