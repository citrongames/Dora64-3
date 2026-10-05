# KaenDan370–371 / KaenHousha372–373

Внесены полные closure генераторов, летящего огня и цепочки сегментов пламени:
Source wait/RNG, birth failures, slot links/unlinks, collision/floor/gravity,
model commands, growth, shrink, fade и retirement. 18 функций переименованы
через каталог с комментариями и доказанными переменными.

Native продолжает реальную scaleX math и её Y/Z copies, Y±5 и opacity68 с
исходной нижней границей0.1. Camera-facing prefix373 выполняется также в
промежуточных кадрах; точная ROM boundary пропускает все entry/FSM/effects.
370 учитывает9 child slots,372 —1,373 — capacity9 и реальный parent7C.
Начальное parent inheritance не превращено в непрерывную attachment.

Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
