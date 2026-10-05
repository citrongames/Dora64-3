# Batti287/Honeycomb288/Batti289 в составе блока2

В основной код включены оба Batti и связанный генератор Honeycomb с пятью
реальными child slots80..88, Source cue27, unlink/death, clip waits и RNG.
Управление AI/counters сохраняет исходный период; model clock и математическое
движение работают по Native cadence.

Короткий height3E остаётся исходным gameplay storage. Только ROM-evidenced SH
арифметические записи захватывают его before/step; конечная Y geometry floor+80
продолжает эту recurrence с clamp0..100. Отдельные Source floor resets отменяют
такое продолжение. Это не преобразование short storage в float и не общий
smooth всех floor setters. Direct heading/RNG/target setters тоже остаются
командами; reviewed yaw increments/shrink/pitch имеют свои equations.

Death scale cap0.1 и XYZ copies сохраняют фактическую Source последовательность.
22 функции получили смысловые имена/описания и scoped pointer aliases. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
