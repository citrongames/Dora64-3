# Boss4 435–445

All eleven types and 57 Source functions are included: ten-phase body,
Suzu, Hitode, Doka, room controller, time hole, time machine, fire ball,
shot, smoke and crash. Circle phase/direction, model-frame speed branches,
folded pitch, jump/landing, reflection, scale, fade and live parent pose
are part of the implementation. Health, RNG, scripts, contacts, births,
room teleports and reward174 keep their actual Source order.

Hitode has a real speed discontinuity at half-frame40: Source changes
from .8*(frame/2-24) to .8*(48-frame/2). Preserve that command; extend
the actual CC-derived ramp in each branch, without a guessed pose delta.
Time-hole XYZ is a birth command, not a parent-follow equation.

Semantic names, descriptions and scoped pointer locals retain exact ROM
provenance. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
