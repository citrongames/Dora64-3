Пачка закрыта пользователем; новый CPU/Windows итог и последняя политика проверок — [Test050](native-regressions-test050.md). Ниже сохранена история анализа и ограничений до проверки.

# Исправления материалов после Test049

Пользователь подтвердил оставшиеся рывки воды/спирали и пропущенный animated sand
второго мира. Это новая пачка после049; тесты/генерация проверочного кода/build
отложены до следующего явного «нет». Игра/renderer/GPU локально не запускались.

## Причины и реализация

- В Source sequences801FD33C/398/6A0/794 кадр2 пропущен. Локальный current->next
  детектор давал двойную скорость раз за цикл. Native теперь читает весь
  однонаправленный цикл, сохраняет его исходную длительность и продолжает одну
  скорость, компенсируя offset текущего Source bitmap. Это Native UV, а не
  ускорение bitmap dispatcher или повтор Source commands.
- Другой reset найден в0877..87E и0462..469:8 кадров по-2px содержат половину32px
  рисунка, последний переход был+14 назад. Реальный age сохраняется на каждом
  Source28C74, сбрасывается на constructor28BC8 и держится при паузе/необновлённом
  канале. Native продолжает scroll через следующую половину tile, независимо
  от числа показанных кадров; Source index/delay/flags остаются исходными.
- Реальный sand второго мира: room descriptor801FE6D0, bitmap list801FD49C,
  sequences801FD478 и801FD484 (7 и8 кадров, segments4/5 и6/7). Frames0239..247
  деформируются и не являются одним translated bitmap. Старый adapter их
  отклонял. Теперь общая world material ветвь создаёт CI4 fractional samples
  в свободном frame bank с собственной palette. Подключены и другие deformation
  families: pool025D..260, blue water01FE..20D, vortices0364..373, waves043F..446,
  colored/pulsing lava02AA..2B5 и прочие actual room sequences.
- Blue water0207/0209 имеют14 entries между16-entry palettes. Native сравнивает
  реальные цвета и совместимый32-byte TLUT, а не количество declared entries
  как material flag. Header high flags по-прежнему обязаны совпадать.
- IA8 material animations02D4..2E3,03CD..3E1,0529..530 также оставались Source FPS.
  Они получают fractional intensity/alpha по отдельным nibbles, сохраняя format,
  payload size и Source RDP loads. CI4 не конвертируется в8KiB RGBA16:64x64 такого
  формата не помещается в original4KiB TMEM. Динамическая CI4 palette ограничена
  исходным числом цветов; на промежуточных кадрах возможна потеря точности цвета.
- Отдельные component phases теперь выбираются по точному segment, а не по
  первому совпавшему pixel pointer. TLUT принадлежит тому же component.
  Graph учитывает GIMO object/vertex DB06 перед builder и in-list DB06, включая
  cached exit state; nested DE и cloned pixel/palette addresses принадлежат ticket.
  Проверка bitmap/hash/cache делается один раз на ticket, повторно на новом frame
  после Source resource reuse. При заполнении tail сохраняется Source fallback.
- Spiral0A46: исправление049 учитывало angle reset, но пропускало simultaneous
  scaleX/Y reset. Native-only common pose correction продолжает recycled branch
  в480<time<482. Допуск требует того же bone, последнего winning scale channel,
  linear4-key cycle с одинаковыми ranges/rates. Arbitrary scaling/Hermite/Source
  samples не меняются. Clip0A48 содержит один обычный angle ramp и этой правки
  не требует; clip0489 не проходит coupled-scale guard.

## Охват и границы evidence

Прочитаны все379 actual room pointers80201A74..80202060, descriptor+08 counted bitmap lists и
descriptor+0C channel templates:67 nonempty room references,25 unique bitmap
tables,35 material sequences. Проверены все пары, включая последние->первые,
и их Source ctor/binder/material call paths. Имена и register roles закреплены
в decomp-symbols.json; ROM-derived report — research/native-material-scroll-audit.json.

1475 старых BMP inventory records и286 candidate sequences не являются runtime
доказательством. Family0564..567 имеет matched assets, но не подтверждённый
animated Source owner; её не выдаём за фактический sand0239..247. Sprite faces,
input/cue/resource/sound decisions остаются Source. Общие exact-scroll branches
распространяются и на actor channels вне world material sampler.

Это законченные правки исходников с Source/ROM evidence, ещё без CPU pre-build
и Windows050. Прежние049 PASS не подтверждают новый код. Проверки после закрытия
пачки должны включить полные циклы с skipped/half-tile frames,14/16 palettes,
actual sand/CI4/IA8 loads и loops, paired segments/nested DB06, paused/rebound
lifetimes, ticket/tail retention и coupled spiral angle/scale. Визуальную
плавность, возможный color quantization/ghosting и отсутствие артефактов проверяет
пользователь на следующем EXE; статический анализ их не подтверждает.

Статический итог новых policies:339 transitions/holds в35 actual sequences —
173 CI4 samples,52 IA8 samples,88 translated steps,26 authored identical holds.
Всё это подключено через общие Source binder/material paths; executable/visual
проверки ещё не проводились. Обычные Source-identical holds не ускоряются.
Standalone texture check потребует native_simulation_time.cpp для нового guard;
новые fixtures/check changes выполняются только после закрытия пачки.
