# Sentoutei366–369

Внесены полные closure всех трёх лодок и shared projectile: constructor,
patrol/recoil/chase/fire, jump/apex, floor/depth limits, contact и death.
Native продолжает реальные speed ramps, bounded yaw/pitch, visual copies,
две независимые death tilt/roll equations и rawyaw approach. ROM table368
явно разобрана:1 jump,2 patrol,3 chase,4 fire,10 death; остальные default.

Фиксированные pose/scale/floor offsets остаются Source setter commands;367
имеет пустой update. Common D2 effect outgoing owners входят в финальный аудит.
23 функции получают semantic names/comments/scopes. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
