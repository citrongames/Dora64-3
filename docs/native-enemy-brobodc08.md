# BroboDC08 348–353

Внесены full Source closures FakeT1/RealT1/T2/T3/Bom/Hob: реальные рождения,
связи/clip sync/контакты, patrol/chase/dash/jump/recoil/fire, probe operands,
взаимные corrections после столкновений, gravity/spin/unlink/retirement.
Native продолжает scalar ramps, реальные turns и copies, все model owners.

RealT1 повторяет только доказанный префикс геометрии. Jump Y delta вычисляется
от immutable Source anchor: Native slices не накапливают одну и ту же поправку.
Для Source decision публикуются actual parent endpoint и velocities, с guard
реальных slot/lifetime. Hob выбирает root/preceding linked part как Source.
Шесть прошлых parent heading equations сохраняются при actual Source shift;
Native продолжает соответствующую историческую equation, не lerp completed poses.
History reset по owner/epoch; Native geometry не сдвигает Source history и не
повторяет collision/counters/clip/sound/RNG/spawns. Collision cross-writes
и linked histories остаются частью обязательной финальной dependency validation.

34 semantic functions/scoped variables added. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
