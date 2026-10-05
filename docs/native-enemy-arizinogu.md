# Arizinogu290–292

Внесены полные Source closure обоих variants и linked projectile: idle/wait,
recoil, wander, атака, рост снаряда, отделение/flight, contact и death/fade.
Native продолжает настоящие physical pitch/yaw, sinusoidal course, scalar ramps,
growth scale copies и accelerating death tilt/Y. Rate58 сначала увеличивается,
затем задаёт оба движения; исходные caps/sector snaps и opacity publication сохранены.

Снаряд повторяет только матричную геометрию linked charge phase, с slot/lifetime
guard. Native не повторяет scale commands, release sound, aiming/clamps или
collision decisions. Parent child0 также учитывает настоящий model lifetime.
18 функций получили semantic names/descriptions/scoped variables через каталог.

При разборе поправлена Source база shrink284: первая запись Y берётся из Z.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
