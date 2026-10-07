# Пачки багов после048

Пачки1–7 вошли в Test049. Пользователь закрыл список ответом «нет»2026-10-07.
31 connected CPU scopes PASS; full-player USER_SKIP. Windows049 сохранён,
прежние файлы сохранены. История ниже описывает исходные причины и отложенные
на тот момент проверки; итоговый охват и ограничения — native-regressions-test049.md.

Причины и общие исправления:

- Вода00DF..00EE: первый64x64 CI4 кадр переставляет palette entries относительно
  остальных15 кадров. Индексы и байты палитры отличаются, но RGBA5551 изображение
  совпадает с точным циклическим сдвигом-4px Y. Старый детектор отклонял переходы
  00EE->00DF и00DF->00E0, отчего сглаживание прерывалось на замыкании. Теперь
  сравниваются все фактически показанные texels, включая alpha и duplicate colors.
  Сигнатуры по-прежнему включают исходную палитру/bitmap, поэтому реальная смена
  цвета инвалидирует кэш. Bitmap/Source palette/indices/durations не меняются.
- Спираль фоновой модели650, model0A45/clip0A46: Original+3 скрывает середину
  углового numeric reset (480,360)->(482,1.50311). Native time481 раньше давал
  180.751555°, теперь361° с сохранением точной скорости (360.751555° для481).
  Общая правка type5 linear channel допустима только при совпадении обоих
  соседних slopes с wrapped rate в пределах2%. Полный0->360 оборот и Hermite
  curves сохраняются. Correction после actual SDK393C4 pose, только Native
  simulation/render; исходные commands/clip flags и Source SDK не ускоряются.
  Порядок конфликтующих channels сохраняет последнее Source назначение оси.
- Диалоговый token-94: offset801B8D88 и direction801B8D08 меняются в draw7DCC.
  Старые guards охватывали другой token-96 и continue arrow. Добавлены обе
  отдельные words для того же общего draw, с сохранением actual Source call
  count, выбранного ответа и input. Это общий путь всех таких диалогов.

Поиск аналогов выполнен статическим разбором ROM, без запуска тестов/игры:
1475 известных BMP записей, все поддерживаемые CI4 color/size families —10
групп (байтовые format2 остаются прежним отдельным случаем). Перестановка
палитры на стыке найдена в одной группе00DF..00EE. Разбор линейных angle curves
в general GIMM выявил такой же reset в clip0489/channel5/bone3:
(84,315)->(96,360)->(98,7.5)->(108,45); его охватывает общая correction.
Отчёт: research/native-authored-cycle-audit.json; это анализ assets, не runtime
reachability или визуальное подтверждение. Таблица0113..0122 пропускает frame2
в Original sequence; эта исходная authored команда не переписана.

Подготовлены, но НЕ запущены: choice-token draw cases для обоих ответов,
направлений и краёв oscillator; palette reindex/loop seam/actual color change;
actual SDK angle pose на reset, Source control, обе banks,20/30 periods и
полный0->360 оборот. Общий pre-build и package/commit/push — после всех пачек.
Новые смысловые имена5 функций и9 register scopes закреплены точными ROM
инструкциями; документация обновляется без ручной правки generated C.


## Пачка2: белые fades при входе/выходе и прочих переходах

Реализовано, без запуска тестов/генерации/EXE. Common alpha не имеет отдельного
renderer cache: actual8002FF64 каждый раз вызывает289D8, читающий801AA660.
Низкая cadence возникала раньше, при выборе native scene policy.

- Image policy разрешала deferred white transition только для scene90/state1,
  отклоняя переход к scene1/state2 и остальные destinations. Убрана зависимость
  eligibility от будущего selected scene/state: пока callbacksC0088/C012C прежние,
  frame assets resident. Progress reset — однократная Source команда, не asset load.
- Count1 раньше отзывал policy сразу после decrement до1, оставляя последнюю
  часть alpha на Source FPS. Теперь fade продолжает до actual280DC; этот hook
  дожидается guest completion/SP retirement/HostRead до смены callbacks.
- World UI fade зависел от D494>1 и отсутствия room request70EC. Это пропускало
  fades при world hold70F0, паузах/scripts, внутри комнаты и в последнем интервале.
  Теперь common active fade допускается для exact world callback pair независимо
  от этих timers. Gameplay/cutscene policies сохраняют приоритет; fallback даёт
  fade presentation, Source decisions/таймеры/паузы не ускоряет.
- Interior BA310 stage1 освобождает actors/resources в BA2B8, stage2 загружает
  их в BA254. Добавлены exact ROM-pinned resource barriers на обе реальные
  границы. B9FDC room barrier и280DC process barrier сохранены. CPU framebuffer
  copies/graphics-disabled/unknown callback pairs по-прежнему не получают Native.
- Проверен статический controller-text branch C012C/BFE80: при активном common
  fade он использует те же resident frames; controller/input update остаётся Source.

Статический каталог всех common begin/hold callers и Source контекст:
research/native-fade-path-audit.json. Мир/сценарии/город/другие уровни, Scene34
карта/title/files/image/selection, девять миниигр и общий миниигровой UI используют
один actual alpha draw path. Поиск caller sites не означает визуальное прохождение.

Подготовлен будущий connected loop прогон: оба actual transition branches1/2,
однократный counted progress-reset boundary, scene selection, последний count1
интервал, distinct alpha и сравнение фактического последнего RDP prim-color со
свежим alpha/RGB каждого build, плюс прежняя проверка resource drain/FIFO/banks.
Ничего из этого пока НЕ запущено. Проверки и билд после явного «нет» пользователя.

Дополнительно подготовлены actual world hold draw BB848 и common fade RDP
проверки для20/30 periods,60/360 schedules, обоих направлений и hold1/10 при
pending room/interior запросах. Сверяются Original fade_tick endpoint и реальные
FA prim-color commands; Source hold/request/exit counters сохраняются. Они также
НЕ запускались. Каталог common begin/hold содержит63 exact ROM call sites.


## Пачка3: карта — мусор под курсором и низкая cadence скролла

Реализовано в исходниках, без тестов/генерации/EXE.

- Native camera equation ошибочно допускала area0..2. ROM задаёт четыре anchors:
  (0,0), (320,0), (320,240), (0,240). Переходы с area3 не получали дробные кадры.
  Теперь допускаются все четыре, в обе стороны и по обеим осям. Index0 easing
  не читается: base3B30 совпадает с четвёртым short anchor; используются indices1..7.
- Уточнён размер таблицы:24 descriptors по152 bytes, sentinel-1 в3780. Прежний
  guard32 был неверен и мог принять соседнюю resource table за descriptors.
- Pinned RT64 drawRect при дробном top добавляет целый texel к V. Старый Native
  adapter двигал E4, оставляя E1=0 и F2 width/rows inclusive; это сдвигало изображение
  на строку и допускало чтение за bitmap. Cursor BA2/BA3 имеет прозрачный нижний
  ряд, тогда как дополнительная строка — чужие bytes, отсюда мигающий мусор.
- Общий adapter курсора и значков компенсирует V bias в E1, задаёт S/T clamp
  до реальных width-1/rows-1 и нормализует последнюю полосу до целого rows extent.
  Последний SDK -1 quarter pixel иначе менял ceil-based UV span при дробном Y.
  Прямоугольник сохраняет полный размер; loaders/palette/chunks не меняются.
  Правка применяется только к reviewed scale1 sequence; Original mode не меняется.
- Аналогичные node bitmaps используют этот же adapter. Background2px strips
  имеют integer screen edges, поэтому fractional-top bias к ним не относится;
  их Native scroll уже меняет E1, общий camera fix охватывает и фон, и значки.

Статическое ROM/renderer evidence: research/native-map-sampling-audit.json.
Подготовлены будущие cases: все четыре areas и special node23; оба периода20/30;
60..360FPS; реальные32x32 BA2/BA3 с poisoned bytes за bitmap; rigid UV slope,
transparent bottom/right edge, clamp, exact unchanged F4/palette/derivatives и
adjacent multi-strip bitmap. Эти проверки НЕ запускались; CPU sampling decoder
по pinned RT64 формулам не заменяет визуальную проверку пользователя.

Смысловые имена camera easing/bitmap handles и register scopes обновлены по ROM.
Тесты, билд и commit/push — после завершения всех пачек и явного «нет» пользователя.


## Пачка4: пункты5/6 — круг персонажей и повороты грибов

2026-10-07. Исправления исходников и статический ROM-разбор; CPU/визуального
подтверждения нет. Тесты, генерация и Windows049 не запускались.

### Повороты KinopperT1

Подтверждён пропуск Native equation в dora3_kinopper_t1_roam_8022D7B4.
При prior counter34=0 Source прибавляет10° к physical heading4, копирует
результат в actor yaw10 и ставит timer7. Это поворот раз в8 Source updates.
Ранее chase speed, bounded visual turns и анимация были подключены, но stores
8022D880/8022D884 этой patrol ветви не были подключены. Точное совпадение
конечных Source кадров не обнаруживает отсутствие промежуточного поворота.

Добавлены обе ROM-qualified точки native_enemy_equation. Физическое направление
и модель получают одну угловую equation с кратчайшим wrap; общий E7F54 projection
потребляет её же для XZ, сохраняя Source endpoint. Timer/clip/переход к chase
и проверки столкновений сохраняют Original частоту. Wall/probe reflection
после поворота остаётся мгновенной командой и отменяет устаревший результат.
Этот helper не читает таблицу заданных точек; наблюдение пользователя относится
к обычному roam в лесу фей, буквальный waypoint path пока не доказан.

Поиск аналогов:184 прямых normalizer/store contexts. Из8 отсутствующих hooks
с малыми буквальными углами один подтверждённый periodic turn исправлен.
Churatto выбирает цель перед уже подключённым bounded12 поворотом; Metabo
делает до9 obstacle probes с±40; Sabotti выбирает player bearing±30;
vortex читает heading родителя−20; Doragonrock задаёт newborn projectile yaw.
Два Boss4 stack stores оказались Y−90 spawn coordinates, а не поворотами.
Основания исключений: research/native-patrol-turn-audit.json. Это статический
охват данного механизма, не доказательство runtime всех патрульных состояний.

### Круг выбора персонажа

Разобраны dora3_character_menu_advance_carousel_801F6870, реальный поиск
доступного персонажа EFE2C и цепочка Source callers. Новый выбор разрешён
только при phaseBF70=0. Для соседнего персонажа phase60 уменьшается через
trunc(phase*ROM0.5):30,15,7,3,1,0. Это6 Source updates включая принятие ввода;
следующий held direction может быть принят в седьмом update. Знаки и переход
0↔5 следуют Original wrap; недоступные персонажи пропускаются реальным EFE2C.

Native policy меню имеет update=nullptr. Дополнительные кадры продолжают
положение двух кругов радиусом120 и возвращают временные поля/матрицы;
выбранный index, input и short phase не продвигаются. Статически зависимости
скорости выбора от Manual FPS не найдено. Скорость не изменена на основании
ощущения; нужен actual Source timing comparison после завершения пачек.
Основания и план: research/native-character-carousel-audit.json.

### Проверки, которые остаются перед билдом

- Kinopper264: настоящий constructor/pool/update/projection, far player,
  initialized patrol и355→5 wrap; промежуточные обе yaw/скорость XZ/позиция,
  timer7..0 и однократные clip commands. Source20/30 и60..360FPS. Сохранить
  exact endpoint replay и соседний265 как контроль другой mushroom ветви.
- Круг: actual EFE2C вместо текущего fixture stub, held direction, оба знака,
  wrap0/5 и locked slots; сравнение index/phase/звука/момента следующего выбора
  с Original, восстановление RAM/context на каждом дополнительном кадре.
- После явного «нет»: полный connected pre-build без часового player proof,
  один Windows build, preservation и commit/push main. Старые PASS не новые.

Автоматическая проверка разрешений отклонила запись новых тестовых фикстур,
сославшись на запрет подготовки проверочного кода до ответа «нет».
Этот скрипт не исполнился; новые фикстуры пачки4 не записаны. Сценарии выше
сохранены как документация. Статический ROM-аудит и runtime правки завершены.
Смысловые описания и9 scoped register roles уточнены по точным ROM words.


## Пачка5: миниигра8 — взрыв без урона и перемещение с Source cadence

2026-10-07. Правки исходников сохранены. Тесты/генерация/EXE не запускались.

Подтверждён разрыв между draw и следующим Source damage update:
dora3_minigame8_spawn_explosion_801C4940 задаёт worldXYZ, но не localXZ.
dora3_minigame8_draw_active_objects_801C4EC8 копирует worldX/Z в local0/8.
Следующий dora3_minigame8_check_explosion_hits_801C7304 вызывает713C, который
читает именно localXZ обоих actors и проверяет distance<75. MinigameFrameScope
откатывал эти draw writes после каждого Native build, стирая единственную
публикацию contact coordinates. У взрыва оставались прежние/нулевые localXZ.

В Native Source completion после motion/spawn теперь один раз сохраняется
точная XZ copy для живых slots10..59 и связывается с world equations как Derived.
Следующий Source contact видит координаты как после Original draw; дополнительные
draw frames по-прежнему возвращают временную позу. Y не изменена: type2 имеет
authored draw height5/30. Hit loop сохраняет свой Original bound59 (slots10..58),
ownerDB immunity, radius75, healthD0-1 и invulnerabilityD8=30. Damage/input/throw/
model/score/spawn не вызываются повторно с Manual FPS; Original mode не изменён.

Для движения участников закреплена точная post-physics worldXYZ→localXYZ связь
в stores6E6C/70/78. После дробного world motion sampler достигнутые Derived body
XYZ и shadowXZ обновляются перед матрицами. Исправлен общий пропуск
publish_derived_float: обновлённая local поза теперь добавляет actor в список
changed_local_actors, как обычный sample. Это необходимо для свежей матрицы и
охватывает уже подключённые followers других миниигр. Source projection/velocity/
gravity/lane limits и commands сохранены; реальное устранение сообщённой низкой
частоты движения ещё требует CPU и визуальной проверки.

Статический поиск draw-copy/Source-read выполнен для direct closures всех9
миниигр; в8 отдельно просмотрены три indirect object handlers. Аналогичный
доказанный Source damage consumer обнаружен в8. В1/2/4 похожие small-offset
loads читают driver/grid/opacity/threshold tables, а не draw-only localXYZ.
В3 direct Source closure не читает эти low coordinates; в5/6/7/9 таких draw-copy
кандидатов не найдено. Это ограниченный ROM-аудит механизма, не runtime proof
всех callbacks. Детали — research/native-minigame-draw-pose-audit.json.

Уточнены Source имена spawn/draw/hit gates/radius damage/pool hits/projection,
6 register scopes и описание root/participant.5350 переименован из ошибочного
driver approach в clamp_participant_lane_velocity: это lane bounds74..301.

После ответа «нет» нужны связанные cases, без запуска сейчас:

- Actual Source4940 birth/4E24 dispatch/4EC8 local copy/7304+713C damage;
  взрыв далеко от нуля, в75/за75, owner immunity, четыре участника, повторные
  Native draws и Original health/invulnerability/feedback/retirement cadence.
- Actual6CEC walk/control/projection/bounds/throw and shadow, реальные Source
  stack801AEAF8, периоды20/30 и Manual60/73/120/144/165/360. Проверять дробные
  world/local positions и matrix translation, Source endpoints и draw restoration.
- Derived local refresh должен помечать matrix dirty; inactive/replaced actor
  не получает старую pose. Связанный контроль Mini2 terrain followers.

Старые intro cases отключали actors, а physics proof вызывал только4300:
они не выполняли Source damage, actual walking и draw-to-update interleaving.
Эти старые PASS не подтверждают новую правку или отсутствие данной регрессии.
Полный connected pre-build без часового player proof и один билд — после «нет».


## Пачка6: пункты7/8 — камеры гонок и самолётов

2026-10-07. Реализация/ROM-аудит сохранены; тесты/генерация/EXE отложены.
Пользователь подтвердил, что зимняя гонка — вариант той же миниигры с4 участниками.

Найден общий дефект в race578C/5908 и planes51B0/532C. Source вычисляет shortest
ошибку между yaw камеры и heading участника через несколько normalized adds,
ветку>180 и деление на15/30 при threshold90. Общий Native recorder снимал только
внешний wrap; вложенная нормализация error всё ещё читала эту же camera yaw.
Affine extraction такого выражения не строила корректную circular recurrence.
Движущаяся цель на границе0/360 меняла знак/числовую ветку подкадра, хотя Source
direction branch была выбрана раньше. При постоянной цели промежуточная ошибка
также не давала корректного damped advance. Конечные Source кадры это не выявляли.

Добавлена явная круговая recurrence C'=C+gain*shortest(T-C). Цель разворачивается
относительно завершённой Source yaw и движется по собственной heading equation;
нормализация не перевыбирает направление в каждом подкадре. Source-selected gain
1/15 или1/30 фиксирован на этот период; exact180 tie выбирает отрицательное
направление как Original. Используется существующее affine continuation,
Source baseline/foreign-write guards и пересчёт eye geometry. Готовые пары поз
не смешиваются, renderer interpolation не включается.

Четыре точных stores подключены в runtime и закреплены ROM/body/word/role в
native-minigame-reviewed.json и production generator. Дополнительных вызовов
dispatcher не добавлено. Follow использует body heading+180, finish — heading
без offset; участники0..3 привязаны к actor1..4. Race dispatcher5B64 и plane54C0
сохраняют phase/finish selection. Зимний вариант охватывается тем же race path.
Distance/pitch/finish flags/input/AI/score/resource commands сохраняют Source.

Поиск аналогов по угловым stores всех9 reviewed modules: четыре camera cases
и11 store candidates в race6944. Последние прибавляют Source integer direction
times ROM rate к driver heading, а не приближаются к circular target error;
существующий additive angle path сохранён. Intro/result cameras других модулей
с additive pitch/distance не получают новый алгоритм без оснований.
Exact bodies/callers/exclusions: research/native-minigame-circular-camera-audit.json.
Смысловые имена/описания четырёх helpers, двух dispatchers и4 register scopes
обновлены. Старые ABI/Original instructions/таблицы адресов не изменены.

После явного «нет» остаётся связанная проверка actual четырех camera helpers,
всех view indices, Source20/30 и60..360FPS; оба направления0/360 и exact180,
threshold90/gain switch, finish handoff, camera eye, distance floors250/300,
Source setters и reload. Для fixed target проверять монотонное signed движение,
для moving heading — непрерывность, а также Source RAM/command restoration.
Никакого нового PASS или визуального подтверждения пока нет.


## Пачка7: пункты9/10 — краткий скачок позы в результатах и после атаки

2026-10-07. Сохранены общий runtime fix,38 exact hook sites, имена и ROM-аудит.
Нового CPU/визуального подтверждения, генерации игрового кода или EXE нет.

Причина связана с концом клипа. Source позирует вT, затем продвигает время на+3;
у unclamped clip счётчик заворачивается и выставляется end pulse. Большинство
обработчиков принимает решение pause/seek/сменить clip на следующем Source
обновлении. Native render в предыдущем ещё advancing интервале применял общий
loop evaluator и успевал показать начало старой анимации. Правка048 сохраняла
paused/zero-rate bone overrides, но не закрывала этот предыдущий промежуток.

Добавлен TerminalPoseIntent только у точных Source end-query владельцев.
Он привязан к зарегистрированному model run, текущему Source step, channels,
descriptors и duration. При положительной скорости render sample после конца
удерживает duration до настоящего Source решения. Seek/rebind/изменение pause
или clamp/reset сбрасывают intent; новая allocation имеет новый run. Guest
counter/end flags/Source decisions не меняются. Bone и morph sampling используют
существующий временный free bank и восстановление; обычные loops и reverse rate
сохраняют прежнее поведение. Готовые позы не смешиваются.

Отдельно разобран Source675C: attack flag20 выбирает clip3; когда current14 и
desired18 равны3, query681C после end обнуляет desired/attack flag, а bind6848
выбирает idle0. Этот one-shot return также получил terminal intent. Result69CC
и8164 содержат по12 end branches для двух таблиц character id0..5; все ветви
включены. В7/9 разобраны общие terminal policies short+4=-1 (pause/seek
trunc(duration)) либо заданный frame. Internal module ID не объявляет доказанным
имя миниигры из игрового меню: общий механизм охватывается по ROM, конкретный
визуальный эффект в сообщённых играх ещё предстоит подтвердить.

Поиск аналогов:41 SDK end query в19 функциях девяти reviewed mini modules.
Включены38 sites, в том числе Source переходы5C/68 в min4 и action2/3->0/4 в8.
Три остальных query (min3:4E50/57B0, min4:42C0) удаляют actor в том же Source
pass после pose/advance и не оставляют живого клипа для render tail; их retirement
сохранён. Terminal hooks закреплены function/body SHA, overlay ROM, JAL word и
component0 delay slot в production generator, который пока не запускался.
Данные/ограничения — research/native-terminal-pose-audit.json. Добавлены/уточнены
имена16 владельцев и19 точных register scopes; ABI/Original instructions сохранены.

После ответа «нет» нужны actual SDK/Source cases всех шести character assets,
обеих result таблиц, attack3->idle0 и других переходов; последние дробные кадры
до/после duration с остатком modulo3, pause/seek/new clip/reallocation/reset,
type10 morph и bone overrides, Source RAM/end flags/command cadence после restore.
Контроль обычных loops и negative-rate clips обязателен. Source20/30 и Manual
60/73/120/144/165/360 проверяются только тогда; старые048 PASS новый fix не доказывают.


## Уточнение pre-build049: camera gain handoff

Связанная проверка actual camera helper на следующем Source кадре выявила
дополнительное расхождение2.93222°: начальная ошибка91°, Source выбирает1/30,
но после этого шага остаток87.9667° уже требует1/15. Старое продолжение удерживало
предыдущий coefficient и не сходилось со следующим Source результатом.
Теперь CircularGain берёт completed camera и target equation в следующей целой
точке, вычисляет только чистую threshold90 ветку и фиксирует gain для интервала.
SDK/dispatcher/commands повторно не выполняются; реальный Source выбор сохранён.
Эта запись уточняет прежнее описание fixed Source-selected gain пачки6.
Проверки всех4 helper/viewports включают actual next-update comparison.

Движущаяся цель выявила второй стык12°: после completed Source camera heading
пересекает180°, а прежний lift сохранял уже устаревшее направление. Теперь
target winding выбирается по той же следующей целой точке, что и gain, и остаётся
одним на дробный интервал. Это pure math, без повторного Source dispatcher или
commands; actual next-helper endpoints и negative180 tie сохранены.
