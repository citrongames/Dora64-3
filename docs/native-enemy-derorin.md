# Derorin308–311 в составе блока2

В основной код внесены полный Source FSM трёх вариантов, создание независимых
копий, прыжки, orbital path310, рост/исчезновение scale, opacity и реальный
falling/fade объект311 из другого ROM overlay. Constructor/contact и shared
states разобраны вместе; Source queries, floor corrections, birth failures,
attachments и collision radius сохраняют исходный порядок.

Синусоидальные scale copies используют physical58; death scale повторяет
abs от округлённого float sine и ROM baseline. Jump scratch58 может затем
переписываться vertical velocity: ранее захваченные scale operands сохраняются
отдельно. Копии308 имеют собственный жизненный цикл и не получают parent pose.
Orbit310 использует фазу до short34+2 и signed spawn coordinates. Тип311 имеет
свой ROM-qualified active callback и publishes opacity до decrement.

21 функция получила смысловое имя и описание. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
