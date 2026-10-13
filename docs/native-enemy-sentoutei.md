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

Test142: см. [подарок Oppi, ТВ и диагностика корабля](oppi-ship-tv-after141.md).

## test143: сборка без журналов; поведение кораблей подтверждено в ROM

Владелец воспроизвёл зависание пары Sentoutei3/368 сначала в Manual,
затем в Original FPS порта и отдельно в оригинальном ROM в эмуляторе.
Оба журнала142 показывают повторную реакцию контакта в chase3:
physical36 остаётся1, после торможения и семи шагов восстановления
скорость продолжает уходить ниже нуля; pitch чередуется0/185, Y растёт.
Исходный код F3A0/F534/F5B8/F600 соответствует этой последовательности.
Это не установленная регрессия Native; игровая логика кораблей НЕ изменена.
Разбор журналов: research/sentoutei-test142-review.json.

По прямой команде владельца test143 собирается без журналов:
Release, CrashLog/GraphicsProfile/Diagnostics OFF, без PDB/CodeView.
Сохранены исправления141/142: рост огня TwinFace, следование подарка Oppi
и левая кайма ТВ. Повторно используется генерация startup-fire141-v1.
Игра, эмулятор, тесты, CPU checks, renderer/GPU агентом не запускались.
Игровая проверка исправлений141/142 ещё не заявляется. Коммит/push ожидают
отдельной команды после проверки. Старые EXE и пользовательские файлы сохраняются.
