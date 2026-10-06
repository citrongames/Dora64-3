# Регрессии после047 — Test048

Исходный отчёт пользователя: карта (качание/переходы/прокрутка вправо и вниз),
белые fades, перевороты самолётов, Source камера интро бомб, краткая чужая поза
после звука в игре с фруктами, ускоренные стрелки предметов. Визуальная проверка
пользовательская; Original инструкции, ROM и сейвы сохраняются.

Установленные причины:

- Stage map BAF64 уменьшает638C;047 смотрел step+1 вместо step-1 и пропускал
  phase3 (size). Новое значение сверяется с actual Original B9F10/BA660 после
  реального уменьшения счётчика. Фон и значки используют один дробный camera phase.
- Bitmap SDK получает целые координаты, после чего его E4 полосы переводятся
  одним остатком10.2. Texture loads/UV/размеры полос сохраняются. Фон BA0CC
  получает остаток камеры через E1, независимо от целых tile loads.
- Planes68AC делает два +180: результирующее добавление360 имеет нулевую
  угловую скорость. До048 составная affine recurrence размазывала360 по времени.
  AngleDelta применяется до выбора Advance, включая аналогичные составные углы.
- Item draw F0CB8 сам меняет два byte counters: guard сохраняет одно изменение
  на исходный Source draw и исходные состояния всех дополнительных кадров.
- UI без update больше не выбирает world preview timestamp: ему нужен текущий
  render clock. Actual loop/fade и полный bomb callback прошли connected pre-build.

Новые проверки включают actual bitmap display lists и actual plane helper,
полный bomb update. Проверки дорогого player не выполняются по прямому запросу.
Эта запись не объявляет симптомы исправленными до завершения проверок/сборки.

Установлен ещё один общий дефект: paused/zero-rate SDK update всё равно
создавал observation, а begin_frame считал его движением и повторно вычислял
весь скелет. Это уничтожало bone overrides, поставленные после SDK pose.
Теперь неподвижная Source pose сохраняется, actual type10 morph обновляется
в свободном банке. Добавлен actual SDK pause/zero-rate тест с override и двумя
банками. Применение общее; точная визуальная связь с сообщённым фруктовым
симптомом ещё должна быть подтверждена пользователем.

Реальный лог047 (1791290327641) подтверждает SP801AEAF8 в minigame update и
первый Source вызов bomb763C после Native acquisition. Прежняя stack_address
брала64KiB вниз и512 bytes вверх: камера801A99F8 попадала в этот интервал,
её loads превращались в константы, поэтому формула не становилась recurrence.
Тесты с SP803F0000 не воспроизводили эту ошибку. Теперь учитывается нижняя
граница действительно достигнутых Source stack frames до исходного SP;
все9 миниигр проверяются при реальном SP, включая полный bomb callback.

Новый actual item draw proof поймал endian отличие: одиночный N64 byte лежит
в swizzled RDRAM по address^3. Byte guards теперь читают/восстанавливают
именно этот байт. Существующие word-aligned snapshots сохраняют прежний layout.

Аналогичные случаи проверены: stack_address используется только в общем
minigame collector (все9 игр/3 общие callback pairs). Угловая нормализация
применена и к простому self-add, и к нескольким накоплениям, и к affine
выражению. Неактивная/нулевая pose обрабатывается общим ModelAnimation, так
что та же причина исправляется в остальных игровых моделях. Draw byte guard
содержит endian поправку; word guards сохраняют свой прежний layout.

CPU диагностика:414 UI cases/332 actual Original endpoints/8682 samples,
реальные chunked texture commands и item draw,187 minigame cases/1425 slices
при фактическом SP. Общий pre-build048:31 scopes PASS, full-player USER_SKIP. Белые fades
прошли actual loop/transition scope; Windows048/package завершены; прежние файлы сохранены.

Аудит3 world-menu draw roots: item3358 содержит единственный временной byte
oscillator F0CB8; weapon5C2C только копирует текущий selection state, character
6F44 не меняет byte/short/float timers. Оба других меню не требуют такого guard.

Ошибки в записи retained provenance/пустых shard были исправлены отдельно
от игры. Результаты готовых независимых проверок не перезапускались. Fresh
enemy scope — moving spider264 с одним worker; static257 не подходит для
общей проверки moving_samples>0. Исторические state coverage/log hashes
сохранены, не подменены новыми PASS.

Windows048 сохранён в общей testbuild папке. SHA256: d9c74a97c0c7e1a0d538a97d463fa13be4594d279d486ef9dc7dc8e6ddb5038b.
Старые EXE/ROM/settings/saves/DLL проверены до и после упаковки;32 dependency
pins и donor unchanged. Код/имена/документы и отчёты сохраняются в main.
Локальный игровой запуск не выполнялся. Визуальное подтверждение пользователя
не заменяется CPU PASS, особенно для фрукта/фильтрации bitmap.
