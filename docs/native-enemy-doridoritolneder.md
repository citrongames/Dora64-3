# Doridori320 и Tolneder321 в составе блока2

Оба полных Source closure перенесены в основной runtime: constructors/contacts,
FSM, movement, recoil, model groups и исчезновение Doridori. Queries, floor
setters, audio, animation bind и подфазы остаются исходными командами.

Doridori получает непрерывные Y/scalar ramps и roll. Его heading является
суммой bounded turn и синусоидальных increments: Native продолжает накопленный
поворот, а не заменяет его одним значением sine. Actual XZ projection происходит
до late heading store, как в Source. Tolneder scalar15+10*sin(phase) продолжен
вместе с его heading; displacement вычисляется по product harmonic sidebands.
На исходном endpoint используются настоящие Source float results.

11 функций получили смысловые имена/описания и scoped variables. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
