## Test-042 передан: первая связанная группа блока2

Сборка042 изgeneration358 сохранена в D:/Games/DoraRecomp3/testbuild после
полного connected CPU набора28 checks.25 типов/24 callback roots прошли
300 frozen/current Original и900 Native schedules; полный player/world suite
завершён на24 настройках. Добавлены152 смысловых имени функций и238 scoped
variable names; документы обновлены. Игра/renderer/GPU локально не запускались.
Все предыдущие файлы, ROM/settings/saves/shared DLL сохранены;32 pins/donor
неизменны. Следующими внутри блока2 остаются остальные семейства, связанные
части/генераторы и боссы; весь блок2 и весь variable FPS ещё не завершены.

# Блок2: враги и активный AI — этап generation358

Пользователь подтвердил исправления test-041 и выбрал блок2. Текущая группа
из25 типов прошла отдельную подключённую CPU-проверку; общий pre-build связей
подсистем завершён. EXE042 собран и передан после28 connected CPU checks. Original таблица содержит
592 типа, общий pool —200 actor slots с индексами30..229. Scene guards теперь
проверяют kind0..591; диапазон actor slots сохранён.

В коде подключены25 типов и24 ROM-qualified callback roots:

| Семейство | Kinds | ROM overlay | Разобранные состояния |
| --- | --- | --- | --- |
| KinopperT1/T2 |264/265|12FD90 .demo_slot1|1/2/10, движение/атака/смерть|
| Pyonger1/2, PyongerTama |266..268|0CEC60 .demo_slot0|1/2/3/10, связанные создание/attachment/flight/removal|
| BroboA100/A101/A103/A104, два снаряда, эффект |341..347|0CEC60 .demo_slot0|hover/patrol/return/oscillate/aim/fire/hit spin/death/projectiles/effect|
| Sabotti1/2/3 |269..271|0D9910 .remaining_slot0_04|1/2/10, pitch/orbit/dive/return/spin/scale collapse/death|
| MiniCar1/2 и связанный дым |359..361|0EDDB0 .level1_slot0|route/chase/turn/stop/resume/death, parent attachment/unlink/fade|
| PatPot и снаряд |362/363|0EDDB0 .level1_slot0|chase/roam/aim/fire/death, XYZ flight/collision/retirement|
| Powan1..4 и эффект превращения |387..391|0EDDB0 .level1_slot0|idle/dialogue/orbit/rise/exit/return, story interaction/transform/present/wave|

`native_enemy.cpp` запускает полный реальный private FSM один раз за actor-local
Source период. Wait counters/RNG/clip/sound/spawn/damage/retirement сохраняют
исходный порядок. На каждом Native интервале продолжаются реальные XYZ equations,
scalar speed, heading/roll и model time. Сохраняются реальные Original proposal
bits в конце периода; дополнительные кадры не повторяют команды AI.
Identity включает kind, даже если два типа разделяют один callback/contact.
Смена типа/состояния/lifetime начинает полный Source период от timestamp команды;
старые clip checkpoints не доставляются новому типу. Внешний motion-only write
перестраивает движение без повторения AI counters/RNG.

Общие уравнения используют разобранные функции
`dora3_object_project_heading_velocity_xz_801E7F54`,
`dora3_object_project_pitch_velocity_y_801E7FC4`,
`dora3_object_project_heading_pitch_velocity_xyz_801E801C`,
`dora3_actor_apply_gravity_probe_vertical_collision_801E810C` и
`dora3_object_step_heading_toward_player_801E88C4`.
E7FC4 — проекция скорости по pitch, не gravity. Только реальный посещённый
E88C4 или явно разобранный математический store задаёт постепенный поворот;
initialization, wall reflections и aiming offsets сохраняют мгновенные setters.
Speed ramp, physical yaw/pitch и XZ/XYZ projection связаны в одном дробном
kick-before-drift уравнении. Суммы sin/cos для меняющихся углов вычисляются по
реальной Source recurrence; момент и float order каждой Source projection
сохраняются. Отдельная slowdown после projection не меняет предыдущую скорость.
Source gravity cap-48 нормализует внешнюю скорость ниже предела мгновенно.

Brobo hover сохраняет исходные sinus phase34+10, ROM radians и base height;
переходная высота продолжает фактическое exponential approach. Реальные
acceleration/deceleration stores связаны с X/Z kick/drift. Sabotti использует
sinus pitchC+10 и Source-equivalent fractional sum для вертикального движения;
смертельный roll±12 и зависимый от private counter visual yaw продолжаются между
исходными командами. Исходные условия clamps/переходов не переписываются.
BroboA103/A104 сохраняют actual ROM jump tables, contact differences, floor/ledge
probes, матричный muzzle offset-24/130/120 и разные условия выстрела18/27.
A104/state2 (Source scalar speed0) сначала меняет turn rate5C на signed5, затем
прибавляет её к total58/yaw4; Native продолжает эту angular recurrence вместе.
Hit spin5, наоборот, прибавляет текущую integer rate38 и только после этого
уменьшает её на4. Намеренные death Y58±10/11-decision shakes остаются Source poses.

Прикреплённый PyongerTama повторно выполняет только разобранный участок Original
matrix/attachment geometry: actor yaw, XYZ translation и fixed offsets0/96/205,
без чтения bone transforms модели. Constructor, counters, animation/shot/collision
commands обходятся; актуальная parent pose сохраняет и XYZ, и heading. Родитель
идентифицируется physics7C и lifetime. Pyonger2/physics80, наоборот, хранит индекс
созданного снаряда; чтение его model time<=0 относится к связанному выстрелу.
Link7C/80 входят в command guard. Unlink7C=-1 запускает настоящую flight command,
а pose-only path никогда не переходит в ветку constructor/clip bind при unlink.
Во время дискретного callback ребёнок читает сохранённый Source endpoint более
раннего родителя; затем Native visible pose восстанавливается. Размер очередного
дробного кадра не меняет входы исходной команды. При полном Original периоде
это совпадает с реальным порядком общего pool.

MiniCar smoke использует собственный pose-only участок: копирует parent XYZ и
visual yaw, без model bones и без изменения physical yaw. Route pointer48,
waypoint42, stop marker3E и smoke link80 являются командами Source FSM.
Намеренные shake poses Brobo/MiniCar на damage/death остаются Source commands;
их private counters не ускоряются дополнительными кадрами.

Powan — сюжетные персонажи, а не обычные противники. Contact mode4 и разрешённые
states, dialogue B922C, story3748/interaction3760, thresholds4/6/8, transformation
ticks35/55 и реальные constructors/destructors сохраняются. Native equations
продолжают actual orbit200/phase42+2, rise phase36+5, vertical arc и yaw wave34+20.
Reverse animation cue0 и rise cue30 используют реальные checkpoint crossings.
Эффект391 продолжает sine scale/opacity; Source Z scale копирует X equation,
включая внешнюю anisotropic scale. Смертельный scale collapse Sabotti3 сохраняет
тот же порядок X-to-Z assignment.

Opacity продолжается только при удержанном разобранном store counter58 и точном
совпадении actual published model alpha. Native воспроизводит actual 3B168
model74/78 и E89CC physics2A flags. Powan публикует после обновления counter,
smoke — до decrement; первый bind остаётся командой. Внешняя alpha/flag запись
снимает продолжение. Неразобранная прозрачность не получает Native equation.

ModelEventCheckpoint накапливает реальные clip advances/end pulses между
решениями AI. Brobo cue18, MiniCar cue36, Powan cue30/reverse0 и Pyonger linked zero cue не зависят только от последнего
дробного кадра; смена clip run/lifetime защищает от устаревших событий.
Внешняя запись pose/velocity/acceleration меняет опорное движение и не повторяет
counter/RNG commands. Настоящая смена state/latch/counter получает новый Source
период от точного command timestamp. BlinkD8 уменьшается на локальной Source
границе; C9 pause обрабатывает общий pool/object clock.

Добавлены152 смысловых имени функций и238 scoped register names с ROM instructions,
включая actual constructors/contacts. Projectile343/344 retirement —state2,
а PyongerTama —state10; contact mode0/1/3 и blink30 закреплены отдельно.
Каталог теперь содержит1001 descriptions,670 semantic aliases и834 register scopes.
ABI symbols, tables/function pointers, ctx storage и reference sources сохранены;
документация использует aliases с прежними адресами в таблице соответствий.

Подключённая CPU-проверка generation358 сравнила300 расписаний frozen/current
Original и900 Native расписаний для25 типов в4 ROM overlays. Проверены72 000
состояний общего пула,638 850 интервалов и столько же zero-dt calls,768 настоящих
контактов. Частоты Native:60/73/120/144/165/360; исходные периоды20/30Hz.
Проверка включает фактические constructors/destructors, связанные дочерние
объекты, model clocks, контакт1/4, pause/resume, внешнюю команду pose и смену
343/344.159 целых Source функций совпали с frozen356 после удаления строго
учтённых approved timing hooks; команды AI не заменены моделями проверки.

Для Source20 сравнивается каждая исходная граница; для Source30 — каждая третья
(100ms), чтобы округление1/3ns не вошло в следующий период. Контексты вызывающей
стороны и zero-dt state проверены на дополнительных кадрах; поля не маскируются.
После принудительного завершения все реальные model/resource allocations
возвращаются к исходному состоянию. Отчёт: research/native-enemy-check.json.

Ресурсы моделей, геометрия комнаты, звук и lookup используют синтетические
данные. Не разобранные scripts/effects возвращают реальный отказ allocation,
с записью команды; весь набор death particles и все ветки каждого FSM этим
прогоном не доказаны. Реальные уровни, renderer/GPU и визуальная плавность
локально не проверялись. Все25 типов и весь блок2 не объявляются полностью
проверенными в игре.

Эта проверка выявила две реальные ошибки переноса. Общий prelude E8D44 сбрасывал
physical2A между дробными кадрами; Native теперь сохраняет фактический результат
callback/E89CC до следующей исходной команды. Это предотвращает потерю
transparency flag у снарядов и ложную отмену opacity equations. В Powan2 orbit
беззнаковый dynamic offset менял знак guest address в MEM_H и мог читать host
память со смещением4GiB; offset теперь явно знаковый. Обе исправленные области
входят в повторный успешный прогон текущей группы.

Остаются остальные ground/flight AI семейства, attached parts/generators и
боссы. Общий connected CPU pre-build для этого крупного этапа выполняется один
раз; после обнаруженных ошибок повторяются только их проверки и зависимости.
Игра/renderer/GPU локально не запускаются. К другому блоку не переходить без выбора.
