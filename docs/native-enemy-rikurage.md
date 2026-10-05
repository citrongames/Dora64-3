# Rikurage336–338

Внесены полные closure обеих медуз и linked light: idle, chase/return,
frame-driven attack, contact, waterband/floor/ceiling recovery, fade/death.
Native продолжает ramps, pitch, late yaw copies, deathtilt/spin, alpha и lightXYZ.
Сохранён scalar scratch перед frame triangle; slope продолжает actual Source
operands. Clip-frame profile/cusp coverage остаётся в финальном аудите модели.

Floor/ceiling velocity kicks не уничтожают ранее потреблённую projection:
runtime сохраняет actual additive correction, проверяет float endpoint и
прибавляет её к discrete drift. FloorY setters остаются instantaneous Source.
18 функций получают semantic names/comments/scopes. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
