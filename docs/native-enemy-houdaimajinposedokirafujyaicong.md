# Houdai382–384 / Majin385–386 / Posedo393–394 / Kirafu395–397 / JyaiCong398

Внесены полные closure пяти связанных групп, включая base/smoke/projectile,
controller/shield, progress/dialogue, contact и retirement. 50 функций получают
semantic names/comments/scoped variables. У394 constructor lake2 и пустые
update/contact scene1 описаны отдельно; inventory включает обоих ROM owners.

Houdai projectile получает yaw10/pitch5 и реальные visual copies до XYZ;
MajinSpirit — настоящую sinusoidal scale с Source +/-9 и original visibility.
Kirafu shake сохраняет signed countdown recurrence, particle scale/speed/fade
и fixed smoke lifetime. При записи final baseline Source отменяет shake.
JyaiCong geometric angular-rate sum управляет настоящими Xsin/Zcos absolute
orbit и visual tangent, сохраняя signed direction и Source clip-end stop/fall.

Linked child/parent lifetimes входят в command guards. Ни base383, ни smoke396
исходно не следуют позиции родителя; выдуманная attachment не добавлена.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
