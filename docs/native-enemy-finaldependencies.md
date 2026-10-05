# Связанные Source endpoints и исторические части

Source endpoint хранится отдельно от текущей видимой позы; временное чтение
родителя дочерним FSM больше не зависит от последнего Native подшага. Это
включает вложенные части и исходные pitch/yaw/roll. Историческая копия простых
привязанных частей хранит настоящую математическую траекторию родителя и
исходное смещение, не изменяемую live позу и не смесь готовых кадров.
Остальная чистая геометрия attachments по-прежнему использует Source prefix.
Убрана последняя лишняя полная копия Period в истории Soundorago.
Для скорости Rikurage добавлены реальные rate/seek/run command dependencies.
Shared actor аргументы получают ограниченные ROM-evidenced читаемые scopes.

Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
