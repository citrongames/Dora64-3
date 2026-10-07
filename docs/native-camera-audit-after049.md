Пачка закрыта пользователем; новый CPU/Windows итог и последняя политика проверок — [Test050](native-regressions-test050.md). Ниже сохранена история анализа и ограничений до проверки.

# Камеры: полный статический аудит после049

Запрос2026-10-07: пройти весь код камеры и найти пропущенные обновления FPS.
Текущая пачка остаётся открытой. Анализ Original ROM/исходников и новые правки
не являются свежими CPU или визуальными проверками.

## Найденные пропуски и исправления

Обычная камера Source BB080 продолжает вызывать DBD58 после player update,
даже когда управление/домен игрока не подходит для native_gameplay_preview.
Общий выбор Native режима ранее требовал одновременно допустимого игрока.
Добавлен независимый frame camera path для стабильного scene1:17 точных Source
наблюдений включают15 реально достигнутых dispatch calls и два auxiliary roots.
Native заимствует полную camera geometry из приватного RDRAM, применяя дробное
время к существующим camera equations; gameplay policy сохраняет приоритет,
а UI path при нём не выполняет камеру повторно. После кадра camera fields
возвращаются Source. Pending room/resource/script/menu owners запрещают этот путь.
Текущая world pose и Source команды не воспроизводятся новым полным callback.

Два отдельных auxiliary paths оставались Source-only:59E80 B5788 и scene1
F96E0. Теперь первый продолжает yaw/pitch/distance всех4 views, второй — manual
camera0. Held input остаётся Source snapshot;9AD0 действительно copied held,
а не pressed edge. Source cut последней камеры >2000->50 не предсказывается
дополнительным кадром. Correct59E80/70A50 module generation участвует в identity.
Это служебные экраны, не новое меню настроек порта.

## Проверенный охват исходников

- Все16 ordinary dispatch entries/15 distinct roots: Manual0/1, transition2,
  scene modes3..6/9..15 и мгновенная геометрия fixed-room7/player-relative8.
 13 timed roots используют накопительное дробное время и Source control anchor.
  Fixed-room copies, minus180 spatial offset и шаги collision-search являются
  геометрией, а не temporal rates: их дополнительное масштабирование ошибочно.
- Scripted camera4 records: target/participant follow, signed angle acceleration,
  distance, explicit eye,3 orbit planes, local target/eye и mode0..3. Interpreter,
  channel enable/up-vector/mode/lifetime commands остаются Source. Derived eye
  mode0/2 и explicit eye mode1/3 уже различаются в cutscene integration.
- Девять mini overlays: intro/follow/finish/result и bomb transition10 components;
  все4 camera rows обновляют SDKeye после достигнутых Motion/Angle equations.
  Race/plane circular targets общие для всех participants/зимнего варианта.
  Static intro camera initialization в остальных минииграх не содержит скорости.
- Stage-map2D camera имеет отдельные2 scalar fields6370/6380 и four-area path,
  с дробными draw rectangles/E1. Selection/items/weapons/character-menu camera
  initializers задают неподвижную позу; actor/model motion не требует движения
  этой фиксированной камеры. Title/files/intro/image не скрывают camera rate.
- SDK2F064 получает текущую pose и пересчитывает eye/up, без собственного time.
  Все вызовы SDK look-at/reflection56DB4/57408 находятся вSDK2F2C8; все его
  Source callers и вызывающие2F064 callback pairs просмотрены. Device/resource
  auxiliary panels используют фиксированную камеру. Их Source fallback
  обслуживает commands/resources, а не пропущенную camera motion.

Инвентаризация включает direct camera references, indexed rows и pointers,
переданные через call delay slots; switch alternatives разобраны отдельно.
Числа и классификация находятся в research/native-camera-coverage-audit.json.
Linear candidate inventory не доказывает отсутствие любых runtime defects.
Новые имена/ограниченные camera register roles закреплены в decomp-symbols.json.

## Следующая проверка после закрытия пачки

Нужны actual-ROM CPU camera/UI/integration scopes: independent camera при
блокировке игрока,17 Source pins, отсутствие повторного обновления при gameplay,
Source restore, profile/module transition, floors/input combinations/4views.
Затем один полный connected prebuild без часового full-player и один Windows
build. Старые049 PASS остаются прежними результатами; здесь тесты не запускались.
Реальную плавность у стен/в катсценах/минииграх подтверждает пользователь в EXE.
