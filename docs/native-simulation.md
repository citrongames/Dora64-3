## Продолжение v193 — сюжетные городские NPC прошли общую проверку

Story100 семей13/14/15/16/17/20/21/22, вторичные модели и связанные kind6,
а также специальные состояния200/201 kind21 подключены к Native.
Все семь formal reports соответствуют generation193 и текущим source SHA:
679 Original functions/regions;1152 actual constructor/profile comparisons,
2304 story Source/1152 Native/768 rejection/15360 exact frozen189
RAM/context/heap/resource/command/model-count comparisons;768 special Source/
192 Native/96 rejection/5280 exact comparisons и288 dense Original counter
samples. Прежние player/weapon/damage/death/recovery/door/surface/world и
городские patrol/dialogue/linked группы проверены в той же общей сборке.
ASan/UBSan/LSan и stability193 прошли на24 настройках20/30Hz и60..360FPS.

Исполняются настоящие конструкторы и descriptor parser. У13/14/16 generic
constructor уже создаёт две model groups;13/14 profile1 прикрепляет group1
к кости19 group0 черезA5F0. A2E8 задаёт флаг группы, а не создаёт модель.
Проверены20-бонная synthetic hierarchy, все профили0/1/2, два размещения в
пуле и настоящее раздельное освобождение моделей/ресурсов. У16/profile0
actual sprite42=1; основная диалоговая проверка теперь покрывает именно его.

Kind6 создаётся первым story100 callback, а не конструктором родителя.
Native принимает лишь настоящие взаимные связи, порядок индексов, живые
model lifetimes, правильный overlay/table callback и согласованные C9.
Связанная семья использует возраст родителя при появлении на73/173мс,
паузе, возврате из Source-состояния и полном пересоздании.

Special201 вычисляет rate5C, но Original не применяет здесь постепенный
возврат. Сохранены его мгновенные yaw snaps и возврат в200 на шестом такте.
Изменение busy/yaw внутри одного периода может пересчитать решение,
но не расходует второй дискретный такт. Контролируемое отключение защиты
поймано dense Original oracle даже на Native60; версия с ошибкой отдельная.

643 описания/308 semantic aliases/402 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI; документация обновлена вместе с
именами. main/HEAD,donor,32 pins и frozen035/source181 сохранены.
Игра/renderer/GPU не запускались; эта проверка использует synthetic assets,
явные sprite2/audio/UI boundaries и настоящий guest lifecycle.

В отдельной candidate194 разбирается slot09 ROM1398A0: актёры24..28,
их scene/story guards, контакты и создаваемые объекты. Прочие уровни,
платформы, script camera/cutscenes, миниигры и2D остаются в работе.
Полный variable FPS не завершён; завершение городского блока не завершает
общую задачу. EXE выпускается только после законченного крупного элемента.

---

## Продолжение v192 — диалоги восьми городских NPC прошли общую проверку

Диалоговые состояния13/14/15/16/17/20/21/22 подключены к Native вместе с
прежними player/weapon/damage/death/recovery/door/surface/world группами.
Все семь formal reports соответствуют generation192 и текущим source SHA:
653 Original functions/regions,3072 Source и2304 Native edge schedules,
384 overlay rejections,4608 actual contact-producer comparisons и20736
exact frozen189 full-RAM/context/heap/resource/command comparisons.
Проверены19712 промежуточных значений обоих углов на24 настройках20/30Hz
и60/73/120/144/165/360FPS. ASan/UBSan/LSan и stability192 прошли.

Поворот к игроку и возврат используют отдельные исходные предложения для
угла актёра и физического угла: Source может изменить оба в одном callback,
а затем вернуть только угол актёра. Сохраняются Original переходы, команды,
клипы, busy-флаг и особенность нулевой скорости возврата. Проверены пауза,
возврат из Source-состояния, настоящее пересоздание модели, внешняя запись
углов, перепривязка клипа, рождение на73/173мс и границы signed wrap.
Контролируемый отрицательный тест с одним общим углом обнаружен даже на60FPS.

633 описания/298 semantic aliases/392 register scopes сохраняют canonical C,
preprocessed tokens и ABI. Документация и ссылки на разобранные функции
обновлены. main/HEAD,donor,32 pins и frozen035/source181 сохранены.
Новый EXE не собирался; игра/renderer/GPU не запускались.

Сюжетные состояния100, дополнительные модели и состояния200/201 ещё проходят
отдельную проверку в candidate193. Полный variable FPS остаётся в работе:
остальные уровни, платформы, script camera/cutscenes, миниигры и2D не завершены.

---

## Продолжение v191 — связанные городские актёры прошли общую проверку

Реальное создание по дескрипторам, распределение связанной пары, контактный
обработчик и раздельное освобождение моделей проверены вместе с прежними
player/weapon/damage/death/recovery/door/surface/world группами:629 Original
functions/regions,48 constructor schedules,1008 Source и720 Native schedules,
288 ownership rejections,308 промежуточных углов,6768 exact frozen189
RAM/context/heap/resource/command comparisons и384 actual contact comparisons.
Все семь formal reports соответствуют generation191 и текущим source SHA;
ASan/UBSan/LSan и stability191 прошли на24 настройках20/30Hz и60..360FPS.

Поворот к игроку и обратный поворот обновляются между исходными тактами,
а команды и переходы сохраняют Original cadence. Детали контакта и диалога
разобраны для linked18/19; story-вариант18/6 использует общий возраст семьи.
Получатель19 читает только окончательную публикацию родителя после base
callback. Внутренние привязки клипов родителя не являются командами ребёнку.
Отрицательная проверка выявила потерю повторной публикации одного и того же
клипа одновременно на Native60 и более высокомFPS: сравнение только между
частотами скрывало ошибку. Счётчик публикаций и сравнение с Original исправили
и проверили этот случай. E656C снимает взаимные ссылки, EA4C освобождает одну
модель; автоматического рекурсивного удаления пары нет.

609 описаний/274 semantic aliases/352 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI. Каталог generation191 содержит
доказательство кандидатной проверки; настоящая общая проверка теперь связана
в research/native-city-linked-check.json. Исправлены отставшие описательные
поля player report с сохранением SHA исходного отчёта, stdout и проверенного
checker source. main/HEAD, donor,32 pins и frozen035/source181 сохранены.
Новый EXE не собирался, игра/renderer/GPU не запускались.

В отдельной candidate192 подготовлены диалоговые состояния восьми других
городских NPC. Их ещё не проверенная реализация не перенесена в main.
Диалоги этих NPC, secondary/story/200 состояния, остальные уровни и платформы,
script camera/cutscenes, миниигры и2D остаются в работе. Полный variable FPS
не завершён; отдельный connected блок не является завершением всей задачи.

---

## Продолжение v190 — восемь городских patrol NPC прошли общую проверку

Полный connected check исполнил611 Original functions/regions вместе с
прежними player/weapon/damage/death/recovery/door/surface/world группами.
1536 Source/1152 Native schedules,192 overlay rejections,1232 intermediate
position samples и11904 exact frozen189 RAM/context/heap/resource/command
comparisons прошли на24 настройках. Все семь formal reports совпали с
generation190 и текущими SHA; ASan/UBSan/LSan и stability190 прошли.
597 описаний/262 semantic aliases/343 register scopes сохраняют exact
canonical C, preprocessed tokens и ABI. Документация синхронизирована;
main/HEAD, donor,32 pins и frozen035/source181 сохранены. Новый EXE не
собирался, игра/renderer/GPU не запускались.

В отдельной кандидатной191 копии реальный descriptor parser, generic
constructor, linked allocator и раздельное освобождение18/19 прошли48
Source schedules на24 настройках. E5DF4 начинает поиск с parent index:
ранний свободный слот не делает leg раньше parent. E656C лишь снимает
взаимные ссылки, а EA4C освобождает одну модель; рекурсивного удаления нет.
Synthetic inactive slots имеют explicit resource owner=-1, потому что
scene_object_init не очищает это поле. Native command-period/visible-yaw
реализация191 подготовлена; её differential proof продолжается и в main
она ещё не перенесена. Dialogue/attached/story состояния остальных NPC,
реальные комнаты/уровни, script camera/cutscenes, миниигры и2D остаются.
Полный variable FPS не завершён.

---

## Продолжение v190 — ожидание и ходьба восьми городских NPC в main

Подключены Native state1/state2 для kinds13/14/15/16/17/20/21/22. Реальный
FSM сохраняет private wait40, одноразовые clip0/1/2 и исходные команды
разворота один раз за Original период; общие перемещение и model time
обновляются на каждом Native интервале. X/Z используют один float baseline
на период с повторным захватом после явной записи позиции/скорости, чтобы
накопленная погрешность не сдвигала точную границу маршрута. Model rebind
не дублирует private counter; новая model lifetime получает новый clock.

Изолированная190d проверка прошла1536 Source/1152 Native/192 overlay rejection,
1232 intermediate position и11904 exact frozen189 Original RAM/context/heap/
resource/command comparisons на24 настройках. Разобраны все восемь разных
осей, границ и direction flags. Negative guard-off даёт count9 вместо4;
внутренние clip-boundary splits тоже не должны ускорять ожидание. Source
snapshot каждые3 периода имеет точное ns-время; floor1/3ns ещё до discrete
end counters, а ceil уже исполняет начало следующего периода. Все clip
commands дополнительно сравниваются по исходным rational timestamps.
Oracle обязан восстанавливать и сравнивать actual clip-channel heap/refs
вместе с RAM, иначе двойной проход некорректно повторно освобождает buffers.

В main597 описаний/262 semantic aliases/343 register scopes; документация
синхронизирована. Полный connected190 check выполняется; последний совпавший
полный архив189 сохранён. Нового EXE нет, frozen035/source181 прежний;
игра/renderer/GPU не запускались. Dialogue/attached/story состояния этих NPC
и связанная18/19 ещё Source-owned. Разбор real descriptor constructors/
attachment ведётся дальше; остальные комнаты/уровни, NPCs/platforms,
script camera/cutscenes, миниигры и2D тоже остаются. Полный variable FPS
не завершён.

---

## Продолжение v189 — диалоговые helpers75/76 и cat77 прошли общую проверку

Полный connected check исполнил545 Original functions вместе с прежними
player/weapon/damage/death/recovery/door/surface/world группами.864 Source,
144 Native schedules,72 отказа overlay и1032 промежуточных наблюдения yaw
новой диалоговой группы прошли на24 настройках. Все семь formal reports
совпали по generation189 receipt и текущим SHA. ASan/UBSan/LSan прошли;
main/HEAD, donor,32 pins и frozen035/source181 сохранены.597 имён/262 aliases
ещё не в main: это отдельно подготовленная кандидатная190 группа городских
patrol callbacks13..17/20..22; её CPU-проверка продолжается. В main остаются
573 описания/238 aliases/335 scopes с точным canonical C и ABI.
Нового EXE нет; игра/renderer/GPU не запускались. Остальные состояния NPC,
связанная модель18/19, комнаты/уровни, script camera/cutscenes, миниигры и2D
ещё не завершены. Полный variable FPS остаётся в работе.

---

## Продолжение v189 — helper75/76 и cat77 в main, focused PASS

Настоящие callbacks/contacts летающих помощников75/76 и кота77 подключены
к Native времени.864 Source/144 Native schedules,72 отказа владения overlay
и1032 промежуточных наблюдения yaw прошли на24 настройках. Проверены реальные
descriptor initializer, достаток/нехватка предметов, принятый/отклонённый ответ,
повторные контакты, byte counters, model clips, покачивание, поворот/возврат,
pause/rebind/external writes, constructors73/173ms, AI suppression/resume
и настоящий destructor. Полный remaining heap/resource references совпадает.

Actual initializer устанавливает pending answer1; итог ответа поступает через
наблюдаемую границу ещё не переведённых dialogue progression/input. Внешний
busy release после rational Source boundary нельзя записывать за долю ns до
неё: только этот after-boundary event flush использует ceil. Обычная геометрия
сохраняет floor. Cat descriptor зависит от выбранного персонажа801B374A;
имена game-counter сохраняют исходное signed-byte переполнение/ограничения.

573 описания/238 смысловых aliases/335 register scopes; names189 подтвердил
точный canonical/preprocessed C и ABI для96 C units. Документация иaddress-map
обновлены. Model/loop/simulation/frame-time/overlay189 прошли; stability189
сохранила main/HEAD, donor,32pins и034. Full connected player189 выполняется;
последний совпавший полный архив188 сохраняется. Windows035/source181 прежний,
новогоEXE нет; игра/renderer/GPU не запускались. Исследуется следующая группа
городских NPC13..22, включая связанную модель18/19. Остальные комнаты/уровни,
NPCs/platforms, script camera/cutscenes, миниигры и2D ещё в работе.
Полный variable FPS не завершён.

---

## Продолжение v188 — возвратный поворот исправлен и прошёл общую CPU-проверку

Новый отрицательный сценарий подтвердил пропуск delay-slot записи yaw между
Native кадрами:350 вместо358.219 на73FPS. Теперь запись сохраняется, а state1
переход остаётся на конце исходного периода.1152 orbital Source schedules,
480 Native schedules и1376 промежуточных наблюдений угла прошли оба
направления и переход через360 градусов. Городские288 Source/96 Native/48
ownership rejection и все прежние player/weapon/damage/death/recovery/door/
surface группы прошли вместе с исправлением. Все семь generation188 отчётов
совпали по receipt и текущим SHA; ASan/UBSan/LSan и stability188 прошли.
562 описания/227 смысловых aliases/330 register scopes сохраняют canonical C,
preprocessed tokens и ABI для96 C units. Документация синхронизирована.

В изолированной build-local подготовке189 настоящие helper75/76 и cat77 прошли
864 Source/144 Native/72 ownership rejection schedules и1032 промежуточных
yaw observations на24 настройках. Исполняются actual contact/descriptor/clip/
answer/counter/destruction цепочки; outcome progression/input и sound execution
остаются наблюдаемыми границами. Каталог189 подготовил573 описания/238 aliases/
335 scopes. Перенос189 в main и его connected check выполняются следующим шагом.
Игра/renderer/GPU не запускались, новыйEXE не собирался; frozen035/source181
сохранён. Другие NPCs/platforms, комнаты/уровни, scripted camera, катсцены,
миниигры и2D ещё в работе. Полный variable FPS не завершён.

---

## Продолжение v187 — городские7/9 прошли общую CPU-проверку

Все семь отчётов player/model/loop/simulation/frame-time/names/overlay совпали
с generation187 и текущими SHA исходников. Полный player check исполнил534
Original functions/regions вместе с прежними группами движения, оружия, урона,
смерти, восстановления, дверей и поверхностей. Новая городская группа прошла
288 Source/96 Native schedules и48 отказов владения overlay. ASan/UBSan/LSan
прошли; runtime overlay guards проверены для49 диапазонов и5 ожидаемых отказов.
Сохранены562 описания/227 смысловых aliases/330 register scopes; canonical C,
ABI и preprocessed tokens для96 C units совпадают. Документация синхронизирована.
main/HEAD, donor,32 dependency pins, frozen Windows035/source181 сохранены.
Новый EXE не собирался; игра/renderer/GPU не запускались.

Отдельный новый сценарий ненулевого возвратного поворота выявил пропуск записи
угла в delay slot на промежуточных Native кадрах orbital71..74. Старые сценарии
этого не покрывали. Исправление и проверки обоих направлений через360 градусов
подготовлены для188; данный архив187 сохраняет предыдущий проверенный объём.
Разбор helper NPC75/76 и cat77 продолжается. Реальные комнаты/уровни, другие
NPCs/platforms, script camera/cutscenes, миниигры и2D ещё не завершены.
Общий variable FPS остаётся в работе.

---

## Продолжение v187 — город7/9 focused PASS, общий connected check выполняется

Городские dora3_city_kind7_state_dispatch_8022D938 и
dora3_city_kind9_state_dispatch_8022DBB0 прошли288 Source schedules и96 Native
off-grid schedules на24 настройках (20/30 Source,60/73/120/144/165/360 render).
48 ownership rejection scenarios подтвердили, что без verified .level1_slot1
объект/модель не обновляются Native, а настоящий Source dispatcher сохраняет
владение. Отдельная проверка исполнила реальные runtime DMA/load/unload guards
для всех49 ROM overlay ranges, подмены того же VRAM, частичной перезаписи,
родительских зависимостей, unload/boot reset и5 ожидаемых отказов81/85.

Исполняются настоящие city contact mode4 producers (и skipped modes0/1/2/3/5),
повторные контакты, подготовка dialogue descriptor/state, pool permission,
player scripted state66, белый fade command, sprite pool bind/delay/release,
16-update retirement и Native constructors на73/173ms. Первая CE0 sprite bind
принадлежит началу Original sprite периода; выход CD4 и события counter — концу.
Проверка heap учитывает ровно реально заменённые Original player model buffers
60/6C/70; все city allocations освобождаются, geometry/остальные allocations
совпадают. Ошибки fixture со статическим B4 contact list, resource id0 и смешением
player/city sprite counters исправлены без снятия resource/destructor assertions.

Теперь562 описания/227 смысловых aliases/330 register scopes; names187 подтвердил
точный canonical round trip, одинаковые preprocessed tokens и C17 syntax для96
C units. Документация имён обновлена. Source/render timing и ABI не переименованы.
Общий player187 и model/loop/simulation/frame-time checks идут; последние полные
совпавшие отчёты/архив185 остаются историческим проверенным checkpoint.
Frozen Windows035/source181 и общие settings/saves сохранены; нового EXE нет.

Контактная collision selection, продвижение/верстка/input диалога и sound queue
ещё не проверены (initializer использует наблюдаемый sound queue failure).
Реальная room construction/DMA, другие NPCs/platforms, уровни, scripted camera,
катсцены, миниигры и2D остаются в работе. Полный variable FPS не завершён.
Игра, renderer и GPU не запускались. Продолжается следующая группа.

---

## Продолжение v186 — городская группа7/9, CPU-проверка выполняется

В текущую main рабочую копию подключены dora3_city_kind7_state_dispatch_8022D938
и dora3_city_kind9_state_dispatch_8022DBB0. Первый сохраняет проекцию скоростей
и переход со сменой спрайта на Source границе; второй —16 Source обновлений,
абсолютный timestamp события15F, остановку пулов/игрока и реальное удаление.
Native ownership требует live callback и verified loaded ROM .level1_slot1:
другой overlay с тем же VRAM не получает эти правила. Новые Native-born модели
получают собственный период, команды после счётчика сохраняют мировое время.
Готовится Source/Native доказательство, feature ещё не объявлен проверенным.

Fixture исполняет реальный sprite constructor, frame delays и actor/model
retirement на синтетических assets; actual metadata kinds7/9 имеет sound0
и drop3, поэтому эта ветвь не создаёт дополнительных drop objects. Добавлены
12 смысловых функций,9 register scopes; теперь554 описания/219 aliases/321 scopes.
Документация имён синхронизирована, names186/connected city check идут.
Последний полный совпавший checkpoint —185 (архив v185-connected-world-objects);
его успешные отчёты нельзя приписывать новым изменениям186. Frozen EXE035/source181
сохранён, новой сборки не делалось. Игра/renderer/GPU не запускались.
Полный variable FPS для остальных NPCs/platforms, уровней, скриптовых камер,
катсцен, миниигр и2D остаётся незавершённым; работа продолжается.

---

## Продолжение v185 — общий связанный CPU PASS

Все шесть свежих отчётов player/model/loop/simulation/frame-time/names совпали
с generation185 и текущими SHA исходников. Полный player check исполнил
515 Original functions/regions и прежние группы движения, камеры, оружия,
1392 lethal/death/respawn,144 post-ready и двери/поверхности вместе с новыми
96 Source/96 Native простых объектов и960 Source/480 Native orbital schedules.
ASan/UBSan/LSan прошли. Проверка стабильности сохранила main/HEAD, donor,
32 dependency pins и034; frozen Windows035/source181 уже сохранён ранее.
542 описания/207 смысловых функций/312 областей переменных; все96 C units
сохраняют canonical/preprocessed C tokens и ABI. Документация синхронизирована.
Игра, окно и GPU не запускались; новая сборкаEXE для этого шага не делалась.

Разбирается следующая городская группа7/9: проекция скоростей, ожидание со
сменой спрайта и событие после16 обновлений с удалением объекта. Новые имена
и код подготовлены отдельно; в данном checkpoint185 их Native ownership нет.
Сюжетная ветвь orbital dialogue, другие active NPCs/platforms, actual room
construction/levels, scripted camera/cutscenes, minigames и2D ещё в работе.
Общий variable FPS не завершён. Архив сохраняет проверенный связанный185.

---

## Продолжение v185 — движение объектов по окружности и собственное время появления

Подключены dora3_orbit_object_kind71_74_update_801EEFC8 и его состояния:
dora3_orbit_object_state1_update_801EECF4 сохраняет круг3°/Source период,
dora3_orbit_object_state2_update_801EEE80 — поворот30° и возврат.
Вертикальная волна10° использует исходный sin/cos и предыдущее значение угла.
dora3_orbit_object_contact_command_801EEC70 проверяет настоящий режим4;
в тесте комната1 исключает неподтверждённую сюжетную ветвь dialogue producer.
Общие перемещения сохраняют полный velocity offset после абсолютного XYZ,
а инкрементальные оси и остальные объекты получают Native dt.

Focused CPU PASS:96 Source/96 Native сценариев простых objects6/80,
960 Source и480 Native сценариев orbital71..74,912 full-RAM Original
сравнений; все24 настройки, ASan/UBSan/LSan. Есть движущаяся цель,
contact/return, AI disabled/resume, pause73/resume173, внешние углы,
rebind и настоящий destructor/constructor с повторным адресом модели.
Новое создание в73/173ms сопоставлено с исходной логикой от момента рождения.

Отрицательные результаты: moving-target diagnostic подавал новую команду после
ceil, который уже начинал следующий период на долю наносекунды. Исправлена
граница подачи команды, допуски не увеличивались. Реальное off-grid создание
показало потерю дробного short-angle на общем Source такте: x=-490.413
против -490.041. Исправлено собственным периодом newborn actor; planner
делит обновления на его точных границах, pause сохраняет его возраст.
Состояние привязано к actor/physics/model/lifetime; следующему pool slot
и player возвращается общий clock. Команды сохраняют абсолютный world timestamp.
Первый birth diagnostic отдельно выявил ошибку восстановления heap в fixture:
не следует перевязывать старую модель перед созданием новой; исправлено без
ослабления проверки освобождений. Source Original ветвь не менялась.

Имена:542 описания,207 смысловых функций,312 областей переменных;
включены проверенные FPR union aliases. Все96 C units имеют точный canonical
round trip, одинаковые preprocessed tokens и проходят C17 syntax проверку.
Model/simulation/frame/loop проверки185 прошли; full connected player185 идёт.
Основной connected player checkpoint пока181; test035/source181 сохранён.
Новый Windows EXE на этот отдельный шаг не собирался. Полный variable FPS
для всей игры ещё не завершён; следующие группы — active city NPCs/platforms,
реальное построение уровней, scripted camera/cutscenes, minigames и2D.

---

## Продолжение v182 — простые active objects6/80

Подключены два дополнительных callback: kind6 однократно включает clip0 вstate1,
kind80 имеет отдельный Original no-op. Движение/model/opacity/timers выполняются
на Native dt через общий dispatcher, live callback identity проверяется.
Focused world CPU PASS:96 Source сравнения с frozen unpatched dispatcher61,
96 off-grid pause73/resume173/rearm273 и912 full-RAM Original сравнения;
все24 settings, ASan/UBSan/LSan. Старый mock Source AI теперь synthetic kind7,
чтобы actual6 проверялся без имитации. Первый diagnostic неверно фильтровал
trace clip entry как70A50; Original core trace8002A328 имеетkind1000. Fixture
исправлен по generated Original, runtime и допуски не ослаблялись.
537 описаний,202 смысловых имён,308 областей переменных; names182/docs PASS.
Полный связанный player checkpoint остаётся181; новое покрытие пока focused world.
Windows035/source181 frozen и сохранён, новогоEXE для этого малого шага нет.
Разобраны функции orbital71..74: circle3°, vertical10°, yaw30°/возврат; перевод
и connected tests продолжаются. Остальная общая цель variable FPS не завершена.

---

## Windows test-035 — verified181, 2026-10-02

Общий EXE после крупного связанного блока действий/урона/восстановления/
смерти/возрождения/дверей/поверхностей скомпилирован MSVC и сохранён:
D:/Games/DoraRecomp3/testbuild/Dora64-3-test-035.exe.
SHA256: d703851e214859d70a4bd074d7989a891cf2e1bb1fe2455ede0825ea4e88ec6b.
Шесть текущих CPU reports, generation181, donor и32pins сверены.
ROM/port settings/saves/shared DLLs/034 сохранили прежние SHA.
Game/window/renderer/GPU не запускались; проверки прохождения не выполнялись.
Windows source frozen181. Разработка продолжается с active objects/AI;
общий variable FPS ещё не завершён. Новый EXE на каждую малую правку не делать.

---

## Продолжение v181 — двери и специальные поверхности: общий CPU PASS

Подключены дверь59 и поверхности60..62 через настоящий producer проверки
поверхности. Native выполняет движение/столкновения/анимацию и вращение;
Source command phases сохраняют выход/вход, счётчики и однократные события.
F5 сохраняет подъём и вращение; F4 — однократный толчок при уходе и исходный
перезапуск клипа во время опоры; F6 — spin и heading45°×orientation.
Door59 реально приостанавливает пулы10..29/30..229, записывает команды камеры,
запускает white fade/выбор следующей сцены или выдаёт room request на count10/16.
При запросе старое движение прекращается до следующего владельца комнаты.

Fresh181:503 Original functions/regions;216 Source surface schedules,
144 off-grid Native constructor/reentry/model-rebind/yaw/exit schedules,
96 door pause/camera-command/fade/room-request schedules, оба camera modes,
оба Source dividers,60/73/120/144/165/360FPS. Все прежние группы, включая
1392 lethal/death/respawn и144 post-ready reacquisition, прошли вместе.
ASan/UBSan/LSan PASS. Player/model/loop/simulation/frame-time/names отчёты
совпадают с181 generation receipt и текущими SHA. Donor/32pins/034 стабильны.

531 function descriptions,196 semantic aliases,307 register-role scopes;
96 generated C units сохраняют точный canonical C/preprocessed tokens и ABI.
Новые роли включают pause диапазона actors, scripted camera commands,
special door target, heading input и surface mode flags. Документация обновлена.

Отрицательный178 выявил повторный F4 clip reset на каждом Native dt: настоящий
producer повторяет entry61 лишь по Source clock. Добавлены guarded ctor phase
и suppression по actor/physics/model lifetime/run/tick. Допуски не расширены.
Door fixture исправлен по Original: F2 включает flight ещё до callback; grounded
doorwayF1/FC. Выбор scene1E/5A изB8ED0 зависит от scene-state, не weapon field.
Full181 выявил остаток death barrier в новом door test; fixture ограничивает
его death-watch, сохраняя реальные handoff проверки. Общая проверка повторена.

Комнаты/модели синтетические. Actual script camera advancement, room DMA/decode/
construction, внешний producer49, active NPC/AI/platforms, остальные сцены,
катсцены/миниигры/меню/2D остаются в работе. Общий variable FPS не завершён.
Проверки не заменяют прохождение. Игру/окно/renderer/GPU не запускала.
Следующий крупный Windows EXE подготавливается после этого connected checkpoint;
034/v130 пока сохранён. Новые ветки/reset/clean/commit/push не выполнялись.

---

## Продолжение v177 — постоянные имена и описания декомпилированного кода

По запросу пользователя понятные имена закреплены в recomp/decomp-symbols.json.
525 функций имеют постоянные описания,190 адресных имён получили смысловые
псевдонимы,305 подтверждённых областей регистров получили имена переменных.
Генератор обновляет определения/вызовы и восстанавливает прежние ABI symbols
через decomp_names.h. Register storage, ROM/overlay identity, function pointers,
Original инструкции и диагностические номера строк сохраняются.
Неизвестные роли не угадываются; один регистр может иметь разные имена по областям.

Fresh177 naming PASS:96 C units, точный canonical round trip и одинаковые
preprocessed C tokens, C17 syntax compilation. Canonical C, declarations,
overlay tables и runtime sources совпадают с проверенным175 checkpoint.
Новые полные player/model/loop/simulation проверки не повторялись ради имён;
их fresh отчёты относятся к175. Frame-time177 и stability177 выполнены отдельно.
Описание роли не означает полное игровое покрытие. Native59..62/room loader
и другие ранее указанные незавершённые границы остаются незавершёнными.

Справочники: [функции](decomp-symbols.md), [переменные](decomp-variable-roles.md).
tools/update_decomp_docs.py обновляет справочники/address-map/старые полные имена
в документации; --check проверяет согласованность. Порядок для будущих разборов
закреплён в AGENTS.md. generated Original C/ROM/resources не добавляются вGit.
main/HEAD/donor32pins/Windows034 сохранены. Никакого EXE/Windows sync/game/GPU.
Общий variable FPS продолжается; этот пункт не является завершением проекта.

---

## Продолжение v175 — смерть, возрождение и повторное подключение персонажа: CPU PASS

Завершена связанная проверка lethal входов32/35/37/40/43/46/47/48/49,
состояний смерти50..56, fade, game-over и освобождения старых моделей.
Actual C9BE0 выполняет создание ресурсов/моделей всех шести персонажей;
после room-ready actual67/Ground возвращает Native обновление новых моделей.
Команды после resource barrier сохраняют rational timestamp; старое движение
отменяется по invocation epoch. Байтовые записи здоровья теперь допускают
настоящие unaligned status pointers. Original20/30 сохраняет поведение.

Fresh175: пять отчётов с совпадающими source/receipt SHA. Full player489
Original routines/regions:1392 connected lethal/death/respawn schedules,
144 post-ready reacquisition плюс все прежние группы. ASan/UBSan/LSan PASS.
Frame-time:655360 Original fade comparisons и12 fractional command settings;
loop: FIFO/two-bank/context/resource barriers; model и simulation PASS.
Комнаты/ресурсы синтетические; actual room DMA/decode/level construction,
FX allocation/visuals и внешний producer49 остаются непроверенными границами.

Это завершённый CPU checkpoint блока, не завершение общего variable FPS.
Двери59/поверхности60..62, active AI/платформы, сценарные камеры, катсцены,
миниигры, меню/2D и реальные уровни остаются в работе. Windows034/v130 frozen;
новогоEXE/Windows sync нет. Игру/окно/renderer/GPU не запускала.
main/HEAD, donor и32 dependency pins сохранены. Исходники175 архивированы.

---

## Продолжениеv174 — смерти/возрождение и lethal входы; полная проверка блока ещё идёт

171 focused PASS:528 Source/Native пары на24 settings, actual50..56,
flight55, все6 healthy51 respawns. Actual C9BE0/C9364/C9794/C93D4/C97E8:
старые модели/списки освобождаются, новые реально создаются; реальные
13-node model trees и команды видимости оружия5..12, resource reference
counts, повторное освобождение и reuse guest heap addresses проверены.
Actual BA084 room request имеет тот же точный rational command timestamp,
что C9BE0, даже после отмены Native session. Original first-state commands
продолжаются; старое movement/model second body больше не возобновляется.

172 focused PASS:тот же528 matrix плюс144 шестиперсонажных post-ready
Original67/Ground -> Native reacquisition trajectories. Pending room request
EC=1/2, secondary request7118 и countdown70F0 не разрешают Native acquisition.
Actual B8FB4 отсчитывает hold; actual C9C58/C97E8/camera reset и полный
Original player/origin/camera update возвращают управление новым моделям.
Room DMA/resource decode/real level construction — явная synthetic boundary;
полная загрузка реальной комнаты этим стендом НЕ подтверждена.
Перед B9FDC/room retirement добавлен bank/resource barrier.

173 Native lethal eligibility расширена на32..49/48; новая матрица включает
actual health loss C0738, damage ctors, real C2978 producer35/48, реакции,
смерть/fade/scene cleanup. Первые проверки — negative, не полный PASS.
173 uncovered:байтовый status at801B361B+2*character ошибочно проверялся как
aligned word pointer, поэтому real health records не допускали healthy
damage/hover Native.174 разделяет byte range и aligned structural pointers;
проверка идёт с настоящими unaligned character records. Health>0 для63
сохраняется; external49 producer по-прежнему не исследован.

Последний полный checkpoint166. Fresh169 frame-time/loop PASS были до
последующих изменений; не использовать как подтверждение174 дерева.
Windows034/v130 frozen, новогоEXE/sync нет; игру/window/renderer/GPU не
запускала. Продолжать общий variable-FPS scope, без ожидания playtests.

---

## Продолжениеv169 — подключение смерти50..56 и точное время fade; блок ещё не завершён

Добавлены Native second/collision/model bodies50..56 и один первый command
closure в Source-периоде. Actual ctors сохраняют new-second/next-first order.
Перед EF5B0/actor cleanup подключён bank/resource barrier, до деструкторов.
Новые команды fade получают rational SourceTimestamp с точностью1/60ns;
sampling render использует момент обновления gameplay, а не более позднюю
wall-clock стоимость подготовки кадра. Barrier сохраняет время до cancel.
Original20/30 вне Native сохраняет исходные функции.

Connected focused169:336 пары Source/Native сценариев50..56, живой другой
персонаж либо game-over,24 settings(20/30Hz,камеры12/2,60/73/120/144/165/360).
466 Original routines. Каждый Native game-over выполняется после подключения,
actual модели реально уничтожаются через actor cleanup. Все промежуточные
счётчики/команды exact; geometry tolerances unchanged; ASan/UBSan/LSan.
Движение и fade после передачи следующей сцене не сравниваются как состояние
уже освобождённого владельца. Банки/очередь/context проверяются отдельно
реальным loop, player fixture имеет явный checked barrier stand-in.

Fresh169 frame-time PASS:655360 Original fade comparisons плюс12 rational
command/reversal settings с ненулевым origin; fresh loop PASS:full Original
message/frame loop,270 white-fade builds, FIFO/two-bank/context resource barrier.
Full-player/model/simulation отчёты169 ещё не выполнялись; последний полный
проверенный checkpoint166. Возрождение51 при ненулевом здоровье и lethal
входы через32..49/48 пока остаются прежними незавершёнными границами.
Никакого EXE/Windows sync/game/window/renderer/GPU;034 остаётся frozen.
Дальше: actual respawn/model recreation, lethal paths и другие системы scope.

---

## Продолжениеv166 — здоровое восстановление49 и управление63, без EXE

Подключена здоровая цепочка control2C=1:49 count21 ->63 clip35/rearm30,
разгон/торможение и поворот actual C8118/C1DE8; A меняет90/270 сразу,
скорость подъёма/спуска10. X/Z используют согласованные kick/drift перед
Original collision, Y от guarded constant-velocity baseline с одним float
округлением. Actual solver/external writes/model run/lifetime/gaps rebases.
B очищает2C, вызывает настоящий fall, landing и полный ground return.

49 first сохраняет входящий count своего Source-периода: собственный
second increment не разрешает ранний переход. Counter guard сохраняет
одно увеличение, actual ctor и внешняя запись reset переустанавливают его.
Повторное вооружение30 не декрементируется хвостом собственного периода.
В fall9 первое чтение actual C0C7C flags сохраняется до следующего Source
периода; дополнительный дробный шаг не запускает landing на долю нс раньше.
Это исправило negative165 state1 vs10 на длительности клипа60 без допусков.

Fresh166:144 Source +144 Native schedules,24 settings:20/30Hz,
камеры12/2,60/73/120/144/165/360FPS. Original comparison через реальные
полные player/world-origin/camera updates; каждая третья Source boundary
имеет точный integer-ns timestamp. Native73/173/273ms ctor/re-hit/rebind/
position-yaw-speed writes, A lift/release, B cancel and complete landing;
21/23 fields, exact state/counters/health/control/command counts. Source49
rearm30 и1, ctor63 stationary/steered, yaw359 and actual re-entry tested.
Все прежние connected группы прошли;435 Original routines/regions;
ASan/UBSan/LSan. Четыре fresh отчёта совпадают с receipt/source SHA.

Первичный внешний producer, который выбирает49, пока не проверен: новые
Native тесты вызывают actual health loss C0738 и actual ctor49 в заданный
момент. Это не утверждение о прохождении этого удара в реальном уровне.
Комната/ресурсы синтетические; настоящий visual/gameplay проверяет пользователь.
Предыдущие границы FX/model3/sprite5 сохраняются. Windows034/v130 EXE
не изменён; нового EXE/Windows sync нет. main/HEAD/donor32deps сохранены.
Игру/окно/renderer/GPU разработчица не запускала. Verified166 source archived.

Полный variable FPS продолжается: смерти/возрождение и прочие действия,
active AI/платформы, сцены, сценарные камеры, катсцены, миниигры и2D.
Не ожидать playtests и не собирать EXE на этот промежуточный шаг.

---

## Продолжениеv163 — специальные действия64..67, без нового EXE

Подключены64: три ветки команд персонажа (0:48/318;5:111/336;прочие:0),
полная анимация/отмена/возврат и настоящая цепочка создания, привязки костей
и освобождения model3 на синтетических ресурсах.65 остановка/отмена;66
поворот по краткому направлению с исходной зоной30 градусов и постоянная
скорость;67 одноразовая настройка и десять исходных шагов восстановления,
скорость5, ground/flight и немедленный возврат по аргументу1. В ветках66/67
полёт использует фиксированную проекцию, без чужой команды ускорения.

Счётчик ожидания обновляется до выбора первого callback, один раз за
Source-период. Собственная смена состояния/клипа не повторяет его и не
сбрасывает раньше времени. Реальный вход из idle298->299->300 проверен.
Независимый reference на дробной границе исполняет Original next Ground
callback и точный участок Source61 C8E58..C8EAC (все21 инструкции проверены).
Арифметика таймера в тесте не дублируется; допуски не ослаблялись.

Fresh163:24 settings —20/30Hz, камеры12/2,60/73/120/144/165/360FPS.
216 Source64/67 +192 Native constructor/idle/full-recovery schedules,
264 Source65/66 +144 Native off-grid command schedules.23 поля, точные
состояния/фазы/счётчики/количество реальных команд; A/B, target/D4, внешние
angle/speed writes, реальная привязка модели между Source-шагами.
Все прежние связанные ground/flight/damage/weapons/projectiles/world/camera
группы прошли.427 Original routines/regions; ASan/UBSan/LSan. Четыре fresh
player/model/loop/simulation отчёта сверены с generation receipt/source SHA.

Комната/модели синтетические; звук, lookup/release, sprite5 и неперенесённые
эффекты инструментированы. Model3 ctor/attachment/destructor исполняются;
настоящие уровни/визуал этим CPU-тестом не подтверждены. Пользователь
подтвердил запуск034 без вылетов, FPS/сцена/длительность не указаны.
Последний EXE034/v130 сохранён неизменным; нового EXE нет. main/HEAD,
донор и32 закреплённые зависимости проверены. Игра/окно/renderer/GPU
разработчицей не запускались. Source checkpoint163 сохранён.

Полный variable FPS остаётся в работе:49/63, смерти и другие действия,
active AI/платформы, сцены, сценарные камеры, катсцены, миниигры и2D.
Работа продолжается без ожидания playtests и без промежуточных EXE.

---

## Продолжениеv152 — здоровый урон32..48 и recovery31, без EXE

Подключены recovery31 clip10/stop/return и waiting46/47:20-count ожидание,
один kick15/yaw reversal/unpause, настоящий fall/landing, clip30 и rearm30.
Constructor period46 не получает ранний first decrement из extra render part.
Общая CC=1 запись сохраняется при подавлении повторного impulse; primary46
clip планируется с rate1, mirror с rate3. Высота с реальным gravity command
вычисляется от guarded Source baseline до Original collision solver, с точным
Original float endpoint. Столкновения/внешние команды/identity/run/time gaps
перебазируют clock. Эпсилонного прилипания и коррекции после solver нет.

Fresh152:414 Original функций,24 settings:20/30Hz, камеры12/2,
60/73/120/144/165/360FPS.48 Source recovery31 и48 waiting46/47,
336 Native real hit/health/rumble/full recovery schedules,144 lethal handoffs.
Все прежние связанные группы ground/flight/weapons/objects/cameras прошли.
Player/model/loop/simulation отчёты сверены с receipt и source SHA. Негативные
149..151 результаты сохранены; допуски сравнений не ослаблялись. Independent
Source46/31 выполняет Original player→world origin→camera, включая steering.

Комната/модели синтетические; звук, ресурсы/спрайты инструментированы.
E5C98/E5BF8 исполняются Original, E6968 — checked failed-allocation boundary:
настоящая жизнь/визуалFX и реальные уровни этой проверкой не подтверждены.
Пользователь подтвердил запуск034 без вылетов; уровень/FPS/длительность не
указаны. Последний EXE034/v130. Новый EXE на этот промежуточный шаг не создан.

Полный variable FPS остаётся в работе: прочие действия/смерти, active AI,
платформы, сцены, сценарные камеры, катсцены, миниигры и2D. Работа продолжается
без ожидания playtests; игра/окно/renderer/GPU разработчицей не запускались.

---

## Продолжениеv148 — связанный здоровый урон32..45/48, без EXE

Завершена CPU-проверка healthy damage32..45 и48 через настоящий удар, импульс,
падение, столкновения, клипы приземления/подъёма и полное восстановление.
Hazard40 сохраняет freeze/hide до count15 и один kick70 на16;43 один kick70.
Собственная pause/unpause команда не разрешает второй шаг счётчика в том же
Source периоде. External run/lifetime replacement по-прежнему сбрасывает guard. После real
collision private translation guard наблюдает actual coordinate, но при unchanged
velocity сохраняет Source candidate вместо накопления integer pushouts. Новый
velocity command после contact/external write rebases сразу; Source interval и
identity/run change также rebases. Итоговая позиция после solver не исправляется.
Контакт берётся из настоящего первого probe каждой фазы. Gravity/drive имеют
Source baseline, X/Z сохраняют precise sums до collision, rearm30 не получает
лишний decrement хвостом своего периода. Original20/30 сохраняет whole steps.

Fresh148:404 Original функции, оба20/30Hz, камеры12/2,60/73/120/144/165/360FPS,
96 hazard Source schedules,192 остальных healthy Source schedules,
288 Native real hit/health/rumble/full recovery schedules и144 lethal handoffs.
Все прежние связанные группы оружия/полёта/объектов/камер прошли. Четыре
fresh отчёта player/model/loop/simulation совпадают с receipt/source SHA.
Сохранён verified source checkpoint148. Ошибки146 own-pause и147 room-edge pushout документированы,
допуски проверок не ослаблены.

Комната/модели синтетические, звук/цвет/ресурсы инструментированы. F3/F8/FA
читаются реальными probes из triangle metadata. E59BC/E5894/E5B20/E5A24
исполняют Original с проверенным отказом allocation E6968; жизнь/визуалFX
и настоящие уровни этим не проверены. Пользователь подтвердил запуск034
без вылетов; уровень/FPS/длительность неизвестны. Последний EXE034/v130.

Полный variable FPS остаётся в работе: дальнейшие состояния персонажа,
смерти, активный AI/платформы, сценовые режимы, сценарные камеры, катсцены,
миниигры и2D. Новый EXE после крупного завершённого элемента. Игра/окно/
renderer/GPU разработчицей не запускались; продолжение без ожидания тестов.

---

## Продолжениеv145 — связанный здоровый урон и восстановление, без EXE

Ground32..39 и flight48 подключены: настоящий импульс, непрерывное падение и
столкновения, приземление, клипы collapse/rise, восстановление движения и rearm30.
Source contact берётся из настоящего C0C7C первого hurt callback каждого периода,
защищён state/actor/physical/model lifetime/run и рациональным временем. Новый
удар не наследует старый grounded contact. Clamp drive сохраняет Source endpoint
и kick/drift без коррекции позиции. Vertical gravity также вычисляется из
guarded Source baseline: даже длинное падение не суммирует округления substeps.
Horizontal X/Z сохраняют precise delta sums до collision; actual contact и
внешняя запись инвалидируют сумму, позиции после solver не корректируются;
новый cooldown30 не уменьшается собственным
Source хвостом. Original whole ticks и20/30FPS сохраняются.

Fresh CPU145: по48 Source schedules flight48/ground35..36/heavy37..39/air32..34;
192 Native real hit schedules в73ms со здоровьем, единственным rumble, импульсом
и полным восстановлением;72 lethal ground и72 lethal flight handoffs до второго
callback/движения/анимации. Вся прежняя связанная матрица оружия/полёта/камеры
прошла: оба dividers20/30Hz, камеры12/2,60/73/120/144/165/360FPS,384 Original
функции, ASan/UBSan/LSan. Четыре свежих отчёта player/model/loop/simulation
совпадают с receipt и текущими хэшами исходников; сохранён source checkpoint145.

Комната/модели синтетические, звук/цвет/ресурсы инструментированы. FA hazard
читается реальными probes из triangle metadata. E59BC/E5894 исполняются Original,
E6968 проверен с отказом allocation: настоящая жизнь/визуалFX не проверены.
Это CPU-проверки, не прохождение. Пользователь подтвердил запуск034 без вылетов;
его уровень/FPS/длительность не указаны. Последний EXE остаётся034/v130.

Полный variable FPS ещё в работе: следующие опасности/смертельные действия,
прочие состояния персонажа, активный AI, платформы, сценовые режимы, сценарные
камеры, катсцены, миниигры и2D. Новый EXE после крупного законченного элемента.
Игра/окно/renderer/GPU не запускались разработчицей; работа продолжается.

---

## Рабочий кандидат136 — реакция на урон в полёте, Windows остаётся034

Для живого персонажа подключается48: скорость/проекция общие с полётом,
но Original C6E0C не вызывает orientation helper C1ADC. Направление удара
сохраняется. Допуск проверяет действующий status pointer и здоровье; смертельное
48 остаётся у Original. Генерация130 повторно используется: ROM/rules не изменены.

Новая CPU-проверка сравнивает полную реакцию, клип и возвращение57 с Original.
Её диагностика продолжается; прежние полные отчёты130 НЕ проверяют candidate136.
Verified source checkpoint130 и Windows034 сохранены, повторный EXE не собран.

Пользователь проверил034: «запустил посмотрел вылетов нет, вроде работает».
Длительность, уровень и выбранный FPS не указаны; это положительный общий запуск,
не подтверждение всех действий/таймингов или полного прохождения. Игру/renderer/GPU
сама разработчица не запускала. Полная цель variable FPS остаётся в работе.

---

## Продолжениеv130 — связанный полёт, воздушная стрельба и передача управления

Подключены состояния20/27/57/58 при включённом полёте. Скорость, ориентация,
проекция и столкновения обновляются по native clock. Пара скорости/смещения
вычисляется из исходных команд; снимок защищён actor/physical/model lifetime/run,
точным вводом/параметрами/контактами и рациональным временем. Число случайных
решений и звуков полёта сохраняет исходный темп. Поворот360→0 сохраняет исходное
представление на границе команды, не создавая ранний скачок.

Воздушное оружие9/10 использует реальный нулевой выстрел и повтор с кадра18,
11/12 — заряд с кадра18. Снаряд получает только время после собственного рождения,
включая B между исходными тактами. Все прежние наземные действия оружия, остановка
и возобновление взаимодействия13, attachment snapshots и27 снарядов сохранены.
Урон переводит57 в исходное действие48; старый native проход прекращается до
следующего callback, движения и анимации.48 пока остаётся у Original.

Fresh CPU v130:96 полных Source flight schedules по24 endpoints,24 смены A/стика
в73/173/267ms,192 Source schedules воздушного оружия с полным pool и освобождением,
96 Native press/release,24 held-B schedules по4 выстрела,72 реальных damage handoffs.
356 исходных функций; оба делителя20/30Hz, камеры12/2,60/73/120/144/165/360FPS.
Проверки player/model/loop/simulation совпадают с receipt и текущими source hashes.
ASan/UBSan/LSan используются CPU-проверками. Модели/комната синтетические;
контакты взаимодействия, ресурсы/спрайты и звук инструментированы. Это не прохождение.

Windows test-034/v130 успешно скомпилирован и сохранён в общей testbuild папке.
EXE SHA256: ed0337e4fcf2c441eccf757bf9043d501257f8157c455a53cf9ca2e69161f23a.
Полный variable FPS НЕ завершён: остальные действия/режимы персонажа, активный AI,
платформы, сценарные камеры, катсцены, миниигры и2D требуют дальнейшей интеграции.
Игра/окно/renderer/GPU не запускались; Original20/30 и Manual60–360 сохранены.

---

## Продолжениеv128 — связанное оружие и pause/morph/attachment, без EXE

Player28/29 weapon1/13 подключены вместе с17/18,21/22,24/25 через реальные
Original commands. Weapon13 останавливает clip18, ждёт окончания interaction105,
снимает pause и возвращает исходный morph source. Source ctor/pool независимо
проверяет точку Native выстрела weapon1/2/4/5 по предыдущему action interval,
включая off-grid B. Снимок защищён epoch/actor/physical/model lifetime/run.

Bounce82 сохраняет непрерывные probes, Source-end surface phase commands,
немедленный actual actor contact и birth-relative начало подъёма после импульса.
Его model-scoped скорость вычисляется от начала Source такта, поэтому накопление
float округлений не откладывает точную границу50→10 на ещё один такт.
Пара скорости/смещения остаётся в той же непрерывной collision scope.
Interaction105 потребляет сохранённое окончание clip на проверенной retirement
границе, даже если raw end pulse уже исчез между Source концами.

Проверкиv128:864 Source moving actions до120 endpoints полного pool,
432 Native press/release schedules,192 независимых attachment comparisons,
48 held-B schedules по4 shots, прежние192 charge actions/96 Native charge inputs.
Original Source pause/busy/morph сравниваются на каждом endpoint для13.
Четыре fresh CPU отчёта совпадают с generation receipt и текущими source hashes;
ASAN/UBSAN, обоих dividers и60/73/120/144/165/360FPS. Синтетические ресурсы,
контакты/звук и комната не заменяют прохождение настоящих уровней.

Остаются special flight states20/27/57/58 и другие действия персонажа,
активные NPC/платформы, другие сцены, сценарные камеры, катсцены, миниигры и2D.
Полный variable FPS не завершён. Последний Windows EXE test-033/v89.
Игра/renderer/GPU не запускались. Следующий EXE после крупного связанного блока.

---

## Продолжениеv121 — движущееся оружие, ввод и повторная стрельба, без EXE

Связанный путь player24/25 теперь подключён вместе с21/22 и17/18. Оружие
2/3/4/5/6/7/8 использует реальные initial commands, контакты, дочерние эффекты
и полное освобождение ресурсов. Native нажатие/отпускание между Source тактами
задаёт фактический момент рождения. Мгновенный cue0 получает elapsed движение
только своей новой allocation lifetime; уже выполненный Source cue0 не повторяется.
Held-B weapon6 выполняет repeat18 на точной границе, разделяя event cursor
с ordinary callback: нет дополнительного выстрела на следующем Native кадре.

Проверкиv121:672 moving Source schedules до120 endpoints полного pool,
336 Native press/release schedules,48 held-B schedules по4 shots; прежние
192 charge actions и96 Native charge B schedules также прошли. Статическая
модель86 проверяется по actual trajectory, animated models по elapsed age.
326 Original функций исполняются в CPU closure. Свежие model/loop/simulation
отчёты совпадают с receipt и хэшами текущих исходников. ASAN/UBSAN прошли.
Синтетические ресурсы/комната и instrumented sound/AI не заменяют прохождение.

Остаются player27/28/29, charge20, Native attachment Source-position reference
для2/4/5, активные NPC/платформы, другие игровые режимы, катсцены, миниигры и2D.
Полный variable FPS ещё не завершён. Последний Windows EXE — test-033/v89.
Игра/renderer/GPU не запускались. Следующий EXE — после крупного связанного блока.

---

## Продолжениеv116 — связанные выстрелы и время от рождения, без EXE

Подключены player21/22 и реальные выстрелы оружия2/4/5 через общий dispatcher,
weapon constructor table и существующее Native обновление всех27 projectile
callbacks. Cue15 выполняется после продвижения модели на точной границе времени.
Начальная команда новых снарядов выполняется один раз; живые объекты не получают
повторного шага. Воздушный выстрел использует исходную позицию крепления до
движения персонажа. Поверхностные преобразования84/88 сохраняют Source endpoint,
а контакт с актором остаётся немедленным. Дочерний89 потребляет завершение клипа
после продвижения модели, включая клипы, заканчивающиеся между Source тактами.

Native нажатиеB между тактами теперь проверено для оружия11/12: shot и длительность
charge отсчитываются от фактического рождения, а не от общей сетки игры.
У projectile отдельный clock; пауза приостанавливает его возраст. Allocation token
модели отделяет повторное использование тех же адресов от прежнего объекта.
Общий clock персонажа/камеры/ввода сохраняется. Raw model end flags не меняют
семантику; отдельное observation позволяет дискретному owner увидеть пересечение
в конце своего такта. Исходный путь Original сохраняет20/30Hz.

Свежие CPU проверкиv116:192 charge action/Source B schedules,96 Native B@73ms
exact shot/birth-relative lifetime schedules и288 moving weapon schedules
(земля/воздух, клипы3/7, оба делителя,60/73/120/144/165/360FPS, две камеры).
Сравнивается весь pool на каждом из70 Source endpoints и полное освобождение
ресурсов; первая Original RAM совпадает. Прежние связанные проверки тоже прошли.
Model, общий loop и simulation отчёты сверены по receipt и текущим source hashes.
Ресурсы/комната синтетические; это не прохождение реального уровня.

Следом остаются weapon states24/25/27/28/29, charge20 и повторные/заряженные
выстрелы; Native B entry оружия2/4/5 тоже требует связанной проверки.
Active AI, катсцены, миниигры и все2D остаются в работе. Полный variable FPS
ещё не завершён. Windows EXE остаётся test-033/v89; новый — после крупного блока.

---

## Продолжениеv110 — игрок17/18 → charge11/12, без EXE

Настоящие наземные/воздушные действия игрока17/18 создают charge101/103 через
full player dispatcher и исходный weapon constructor table. Native owner
потребляет frame18 после движения и model advance. Интервал делится у frame18,
поэтому constructor вызывается ровно на исходном времени при произвольном FPS.
Pose acquisition игрока17/18 восстанавливает observation до Original counter.

192 joint CPU расписания: два оружия, земля/воздух, direct action init либо
реальная кнопкаB в Source entry с последующим удержанием;24 FPS/divider/camera
комбинации. Проверяется полная RAM первого Original прохода, единственный
constructor в том же времени, birth clock0, длительность charge8 исходных
тактов и полное освобождение parent/converted effect ресурсов. Общие animation,
loop и simulation проверки тоже свежиеv110. Синтетические room/model assets;
это не прохождение уровня. Native input edge внутри уже активного оружия
и остальные weapon player states ещё требуют подключённой проверки.

Все27 projectile callbacks уже имеют reviewed Native owner. Active AI,
катсцены, миниигры и все2D остаются в работе; полный variable FPS не завершён.
Windows EXE остаётся test-033/v89. Следующий EXE — после крупного связанного блока.

---

## Продолжениеv109 — interaction105, без EXE

Подключены все27 projectile callbacks scene1. Interaction105 выполняет
attachment и пространственное выдвижение на каждом Native update; удерживаемый
NPC получает текущую позицию. Reach использует реальные ключи Original через
три кадра анимации. Phase/seek/spawn/retirement остаются однократными командами
Source endpoints. Захват хранит actor/physics/model и allocation lifetime;
переиспользованный NPC не наследует старый захват. Source commands после AI
восстанавливают удержание в том же времени, не продвигая clock повторно.

120 joint CPU расписаний покрывают свободное действие, удержание, seek/spawn,
phase1→2 и реальную phase0 capture branch с инструментированными contact flags.
Сравниваются Original/v61 по полной RAM первого прохода и полям каждого такта;
54 промежуточных точки удержания за расписание проверяются по геометрии реальных
Original callbacks.48 live births включают полное освобождение parent/child,
сохранение commanded pose без dt до рождения и same-address NPC recreation.
Функции Original движения игрока участвуют в сравнении. Все четыре проверки
свежиеv109, без игры/GPU; Model component отдельно проверяет outside-clip seek.

Это CPU покрытие с синтетическими ресурсами/комнатой и paused NPC; реальные
weapon player states, active AI, катсцены, миниигры и все2D ещё в работе.
Общий variable FPS не завершён. Windows EXE остаётся test-033/v89; новый EXE
будет после крупного связанного блока, а не после отдельного callback.

---

## Продолжениеv107 — траектория самонаведения86, без EXE

Криволинейное движение86 вычисляется на каждом Native-интервале от исходного
состояния такта. Коррекция кривизны исчезает на Source endpoint, сохраняя
перемещение Original. Выбор цели и знак angular command используют исходный
наклон/курс и прогнозируемую конечную точку; текущие xyz после чистого запроса
восстанавливаются. Новый базовый наклон вводится плавно, без скачка на границе
Source такта. Clock движения привязан к allocation модели: pause gap, внешняя
команда, acquisition, partial heap reset и reset системы отменяют старую выборку.

384 CPU расписания покрывают два персонажа, нулевую и штатную скорость25/7,
обычную/почти выровненную цель, поворот через360 и отсутствие цели. Движущиеся
варианты проверяются6 Source тактов; stationary —102, включая конец tracking
window4..94. На каждом такте сравниваются8 полей с независимым Original/v61,
на четвертях — xyz по независимой кривой из реальных Original projections.
На каждом Native update ограничена скорость изменения наклона; реальные hit87,
полное удаление parent/child и освобождение ресурсов тоже проверены. Цели
статические; движущиеся AI и реальные уровни ещё не подтверждены.

Все четыре CPU checks свежиеv107, без игры/GPU. Native callbacks26 из27;
26 families имеют описанное CPU покрытие. Interaction105, старшие действия
игрока, active AI, катсцены, миниигры и все2D остаются в работе.
Windows EXE по-прежнему test-033/v89; новая сборка не делалась.

---

## Продолжениеv105 — charge101/103, общий birth boundary, без EXE

Подключены оба заряда: real constructor/resource lists, привязка к текущему
персонажу, дробный рост радиуса и opacity, phase34+3, one-shot conversion в
102/104 с rumble и полным retirement.48 acquired-Original расписаний сравнивают
все Source endpoints сv61 вместе с настоящим движением игрока. Ещё48 live-birth
расписаний вызывают реальный constructor во время Native player callback:
начальные команды выполняются один раз, model clock=0, phase21/radius10,
conversion через8 Source ticks от рождения, полное освобождение ресурсов.

Снимок projectile pool перенесён перед всеми producers, включая игрока: новый
actor не получает dt до рождения. Birth acquisition одного model сохраняет
scalar clocks остальных владельцев. Original/native-born opacity provenance
различается; компонент проверяет отсутствие overshoot и cache preservation.
Все четыре CPU checks свежиеv105: joint gameplay, model, loop, simulation.
Игра/GPU/Windows-сборка не запускались; EXE по-прежнему test-033/v89.
Активных projectile callbacks26 из27; полностью проверены25 families.86 ещё
требует moving curved trajectory;105 — frame-dependent reach/seek/NPC drag.
Старшие действия игрока, active AI, катсцены, миниигры и все2D не завершены.

---

## Продолжениеv102 — clocks самонаведения86, без EXE

Реальные ctor/callback86, выбор цели, oscillator36, tracking34, yaw1/1.5 и
pitch2 переведены на общий Native timeline. Дробная фаза short сохраняется
на время жизни модели; направление angular command фиксируется на текущий
исходный тик, а движение угла продолжается дробно.48 stationary tracking/
impact/retirement расписаний совпадают по счётчикам и базовым углам сv61;
joint прежних сценариев проходит. Model CPU дополнительно подтверждает12
oscillator и12 angular-command расписаний, wrap и partial heap invalidation.
Криволинейное движение86 с ненулевой скоростью ещё НЕ проверено; это частичная
интеграция86, не завершённая projectile family. Полностью проверены23 families,
активных Native callbacks24. Charge101/103 и interaction105 ещё не подключены.
Loop/simulation отдельный PASSv99; Windows/игра/GPU не запускались, EXE033/v89.

---

## Продолжениеv100 — отскок82 и переход82→83, без EXE

Подключён82: настоящие constructor, gravity, spatial probes, мгновенная смена
скорости при отскоке, создание83 и удаление родителя.24 расписания проверяют
Original-pass по всей RAM, исходный участок траектории по независимомуv61,
дальнейший полёт от реального момента отскока, единственный conversion/rumble,
нулевой clock ребёнка при рождении и полный четырёхстадийный lifetime83.
Joint player/world/projectile CPU check проходит вv100. Model/loop/simulation
последний отдельный PASSv99; повторно их на этом небольшом шаге не запускали.
Native ownership23 из27 projectile callbacks. Homing86, charge101/103 и
interaction105 остаются в работе. Игра/GPU не запускались; EXE test-033/v89.

---

## Продолжениеv99 — четырёхстадийный эффект83, без EXE

Подключён83 с четырьмя настоящими жизненными циклами модели и тремя bind
переходами. Смена стадии выполняется сразу после окончания клипа; новая модель
начинает с нулевого времени и не получает интервал до её создания.24 расписания
сравнивают положение, скорость, фазу, mask и alpha на каждом Source endpoint,
вплоть до полного удаления, с независимым Original/v61. Original-pass также
совпадает по полной RAM. Предыдущие цепочки, эффекты, игрок/камера и CPU loop/
model/simulation проверки проходят. Native ownership22 из27 projectile callbacks;
отскок82, homing86, charge101/103, interaction105 ещё в работе. Игра/GPU не
запускались. Последний EXE остаётся test-033/v89, Windows-сборка не делалась.

---

## Продолжениеv97 — падение, попадание и цепные эффекты, без EXE

Подключены84/88/89: действительная Native-гравитация, геометрические проверки,
попадание84→85 и цепочка88→89 с пятью звеньями.72 расписания проверяют восемь
Source endpoints по независимому Original/v61, единственное попадание/команду
отдачи, создание звеньев, отсутствие обновления до рождения и полное освобождение
моделей/physics/resource references. Таймер34 у89 меняется только на Source
endpoint; пространственное расширение34 у88 на10 при попадании не масштабируется.
Всего native owners21 из27 projectile callbacks, остальные семьи и весь FPS
продолжаются. CPU-проверки игрока/камеры, предыдущих эффектов, loop, simulation
и моделей проходят. Игра/GPU не запускались. Последний EXE test-033/v89.

---

## Продолжениеv96 — рост, вращение и затухание, без нового EXE

Добавлены типы85/87/90: размер и прозрачность меняются на каждом native
обновлении, вращение сохраняет25 градусов за исходный игровой шаг. Начало
затухания, возраст и удаление сохраняют исходное игровое время.72 расписания
при60/73/120/144/165/360FPS, обоих делителях и двух режимах камеры сравнивают
состояние на каждом Source-тикe с независимым нетронутым Original/v61.
Проверены168 цепочек полёта,216 конечных эффектов,912 common-world полных RAM
сравнений, игрок и камера.12 отдельных расписаний проверяют точные float
значения затухания и удаление, паузу, внешние записи и смену жизни модели.
Проверки только CPU, с ASan/UBSan/LSan; игра и GPU не запускались.
Native ownership сейчас18 из27 projectile callbacks; остальные семьи и
полный перевод игры ещё не завершены. Последний EXE остаётся test-033/v89;
новые Windows-сборки только после большого завершённого связанного блока.

---

## Продолжениеv93 — полёт и попадание, без нового EXE

Типы91/93/95/97/99/107 соединены с девятью конечными эффектами на общей шкале
времени. Проверены168 расписаний полёта/паузы/попадания/создания/освобождения,
включая переключение скорости107 с25 на30 через настоящее окончание клипа и
bind следующего. Original-проход сравнивается с нетронутымv61 по полной RAM.
Новорождённый эффект исключён из предшествующего интервала: в scratch-контроле
без защиты его clock сразу стал1.5 вместо0 при60FPS. После исправления время
жизни от рождения совпадает при60/73/120/144/165/360FPS и обоих делителях.
Настоящие constructors, модели, списки ресурсов и удаление выполняются; lookup/
release ресурсов и команда создания спрайтов наблюдательные на синтетических
данных. Это CPU-проверка, игра/GPU не запускались. Последний EXE test-033/v89.
Полный FPS и весь блок27 типов снарядов ещё в работе; далее рост/затухание,
цепочки, homing/charge/interaction, затем следующие части общей задачи.

---

## Рабочее продолжение 2026-09-30 — v93, эффекты завершения

В общий native timeline подключены девять конечных эффектов пула10..29:
типы92/94/96/98/100/102/104/106/108. Их реальные обработчики ждут завершения
клипа и один раз освобождают модель, актёр и physics. Original-пропуск разделён
по актёрам; остальные типы снарядов пока остаются исходными.

Передача существующей модели учитывает последнюю рассчитанную Original-позу.
Исходная функция сначала строит позу, затем прибавляет время; без этого учёта
эффект при смене владельца терял один исходный кадр жизни. Acquisition разрешён
только для проверенных общих объектов и этих эффектов, при свежем наблюдении
и совпадении клипа, каналов, скорости и времени. Действия игрока с кадровыми
событиями не включены в этот контракт.

CPU:216 времён жизни/удалений эффектов сравнены с нетронутым Original801D10D8,
912 полных RAM-сравнений общих объектов,24 расписания движения и48 разворота,
обе исходные частоты и60/73/120/144/165/360FPS. Model и общий loop прошли;
освобождение и контекст проверены, игра/GPU не запускались. Ресурсы и комната
синтетические, это не пользовательское прохождение. test-033/v89 остаётся
последним EXE; v93 ещё не компилировалась для Windows. Полный FPS не завершён:
полёт/попадания остальных снарядов, действия игрока17+, AI, катсцены, миниигры
и оставшееся2D продолжаются. EXE только после крупного связанного блока.

---

## Рабочее продолжение 2026-09-30 — v92, нового EXE нет

Последняя Windows-сборка остаётся test-033/v89; пользователь подтвердил, что
дрожание в городе ушло. Новый порядок: EXE только после завершения крупного
связанного блока, без сборок на каждую промежуточную правку.

В рабочем main допуск игрока расширен до состояний1..16. Поворот назад,
торможение, завершение клипа и переходы12→13/15→16 согласованы с исходным временем.
Общий диспетчер мира801E8BDC теперь делит владение по актёрам: 38 проверенных
типов с пустым обработчиком801E5D84 получают настоящее native движение,
прозрачность, модельные часы и исходную частоту мигания/спрайтов. Остальной AI
пока обновляется исходным путём. Native-проход сохраняет контактные списки.
Перед командами исходного world update вычисляется прошедший интервал, поэтому
новая скорость и появление/исчезновение объекта не применяются задним числом.
Границы клипов учитывают только действительно обновляемые модели и их текущую
скорость, включая новые клипы и исключая остановленные объекты.

CPU:24 совместных расписания игрока/мира и48 разворота/торможения; обе исходные
частоты,60/73/120/144/165/360FPS, обычная камера12 и городская2. Проверены реальные
модельные/flipbook функции, смена скорости, появление/исчезновение, исходные
счётчики, контактные списки, CPU-контекст и освобождение моделей. Original RAM
совпадает с исходным801E8BDC для всех38 типов (912 сравнений). Model/loop/time
проверки тоже прошли; ASan/UBSan/LSan там, где предусмотрены проверками.
Синтетическая комната/ресурсы и наблюдательные sound/AI endpoints ограничивают
этот результат: это не прохождение и не подтверждение всей логики NPC.

Полный variable FPS ещё НЕ завершён. Далее — снаряды/эффекты пула10..29,
действия игрока17+, активный AI всех overlay, катсцены, миниигры и остальное2D.
Игра/окно/renderer/GPU не запускались. Рабочая v92 проверена на CPU; её Windows
компиляция ещё не выполнялась. Старый windows-build-check.json относится к033.

---

## Продолжение 2026-09-30 — test-033 (генерация v89)

Исправлена пропущенная публикация точки отсчёта отображения мира после native
движения игрока. В test-032 она оставалась в исходном world update20/30Hz,
а персонаж и относительная камера уже обновлялись чаще. Пользователь сообщил
о дрожании персонажа и3D-домов в городе, сильнее вдали от камеры. Его runtime.log
1790765643377 подтвердил test-032, Manual60 и городской обработчик801D7294.
CPU-проверка до исправления воспроизвела разность2.25 единицы между позицией
игрока и render origin на13-м кадре60FPS. Теперь после движения вызывается
оригинальный801BBDDC до камеры; координаты общего кадра согласованы каждый шаг.
Пользователь подтвердил: в test-033 дрожание ушло. Это подтверждение его
проверки города; игру/renderer/GPU при разработке не запускали.

Допуск player preview расширен на состояния1..10: ходьба/бег, прыжок, падение,
приземление. Модельные команды теперь проверяются в той же CPU-цепочке, что
ввод, полный player dispatcher, поиск пола, drive, контакт и камера, вместо
подмены model endpoints.182 оригинальные функции,24 расписания: оба исходных
делителя,60/73/120/144/165/360FPS, камеры12 и городская2. Удержание A даёт один
прыжок и одно приземление; позы свободного банка восстанавливают игровые кости.
Синтетические модели/комната и наблюдательные sound/sprite endpoints остаются
границами проверки. Это не прохождение уровня. Общий loop/barrier тоже проверен
после изменения; остальные актуальные CPU-отчёты сохранены без лишнего повторения.

Остальные действия, NPC/объекты, катсцены, миниигры и меню/2D ещё требуют общего
перевода. Полная цель variable FPS НЕ завершена. Original20/30FPS и существующий
Manual60–360 сохранены; RT64 interpolation нет. Название policy в старом логе
scene1-ground-preview оставлено историческим; реальный допуск определяется кодом.

---

## Продолжение 2026-09-30 — v89 / test-032

Подключён полный обработчик наземного персонажа в состояниях1..5: свежий ввод,
решения состояний, скорость, столкновения, post-contact, часы моделей и копирование
трансформации спутника. Старый VI больше не выполняет этого игрока второй раз.
Общий VI продолжает обслуживать контроллеры, звук и ещё не переведённый мир.

Новый ввод хранит время VI-образцов и делит native-интервал на их границах.
Короткие нажатия расходуются один раз; при возврате в Original потреблённые
нажатия не повторяются, непотреблённые сохраняются. Именно CPU-проверка полного
обработчика выявила ошибку применения нового стика задним числом: при73FPS
траектория отличалась от60FPS на6.08 единицы. Очередь устранила расхождение.

Счётчики неуязвимости/максимальной скорости/бездействия/паузы действия и звуки
шагов привязаны к исходному игровому времени. Повторная установка неизменного
флага паузы/конца клипа не начинает часы заново. Native-поза вычисляется по
актуальному времени клипа в свободном банке с восстановлением игровых костей.

Смена сцены отменяет остаток накопленного интервала, включая вложенный вызов
камеры. Барьер ресурсов сохраняет очередь посторонних сообщений и ждёт обе
кадровые банки, позднее завершение SP и HostRead.

CPU-проверки: полный наземный обработчик с реальным поиском пола/объектов,
ввод, анимации, совместная физика/камера и общий loop прошли. Для наземного
обработчика и ввода проверены20/30Hz и60/73/120/144/165/360FPS. Исходные
анимационные кривые проверены отдельно; звук/model/sprite endpoints в полном
сценарии инструментированы. Синтетическая комната не заменяет прохождение.

Полный variable FPS всей игры НЕ завершён: остальные действия, NPC/объекты,
катсцены, миниигры и меню/2D ещё требуют подключения и проверки. При выходе
из допуска возвращается Original. Процесс разработки продолжается после EXE.
Игра, окно, renderer и GPU локально не запускались. Новых настроек и RT64
interpolation нет; ROM, port_*.json и saves при упаковке сохраняются.

---

История предыдущих этапов (ниже):

# Общая симуляция и сценарная камера — работа продолжается

2026-09-29, locked N3DJ rev0, генерация v71. Полный FPS не завершён;
эти continuous closures ещё не подключены к живому update всех сцен.

## Общий подход

`SimulationTimeline` разделяет реально прошедшее время на части до исходных
границ20/30Hz. Непрерывные величины обновляются на каждом выбранном шаге,
одноразовые команды и целочисленные длительности исполняются по своей шкале.
Остаток точный целочисленный;60/73/120/144/165/360FPS не теряют событий.
Original вне `SimulationScope` получает исходные операнды инструкций.
Ускорение/скорость/пространственная коррекция имеют разные размерности.

72 адресных правила в `recomp/native-timing-rules.json` проверяют ROM,
хэш всей исходной функции, слово инструкции и delay slot, когда он затронут.
Это не автоматический множитель всех float-инструкций. Рекуррентные записи
в других функциях пока только кандидаты: среди них индексы, указатели и геометрия.

Постоянное ускорение проверено как непрерывное продолжение исходного
`v += a; x += v`: displacement = new_v*h + a*h*(1-h)/2.
Обычный `x += new_v*h` меняет траекторию при разделении шага. Ограничение
скорости, столкновения и события требуют дополнительного согласования;
проверка отдельного ядра не доказывает полный игровой интегратор.

## Сценарная камера

Root801E0760:4 контроллера802183D0/stride1B8,4 камеры801A99F8/stride34,
режимы0..3. Разобраны timer closures801DF854/DF910/DF9D8,
target/angle/distance/eye updates, orbitDFCD8/DFFEC и localE0324/E0518.
Исходная тригонометрия и статические геометрические смещения сохранены.
Timer boundaries сохраняют исходный порядок decrement/test.

Обнаружено: повторное применение follow offset и фиксированного порядка
вращений при каждом маленьком шаге меняет траекторию. Конкретный отрицательный
пример: eyeY в mode3 при orbit channel4/duration2,7.38615 против19.3661.
`ScriptCameraTimeline` вычисляет проверенные уравнения на накопленном времени
текущего тика от его начального состояния. Это вычисление уравнений движения,
не смешивание предыдущей/следующей готовой позы и не RT64 interpolation.
На границе коммитится исходный результат; далее создаётся новый начальный state.
Сценарные изменения должны происходить на границе или с явным reset/rebase.
Обычная gameplay camera801DBD58 — другая система, ещё требует перевода.

CPU-проверка полного closure с исходными ROM constants: Original/full-step
whole-RAM differential, четыре режима и камеры, отдельные orbit/local каналы,
совместное вращение, duration0/1/2/7/40,20/30Hz, шесть FPS. ASan/LSan;
UBSan на новом C++. Отдельные no-op ветви режима не считаются самостоятельным
доказательством покрытия. Внутренние координаты проверяются вместе с output;
первые тесты только output не обнаруживали дефект follow offset.

Не запускались игра/renderer/GPU. Проверка модели не заменяет прохождение.
Следующие интеграционные области: ordinary camera, player/actors с контактами,
2D writers, mini-games, дискретные сценарии и общая граница освобождения памяти.
Наличие этих оставшихся работ не является причиной завершать текущую задачу.


## v76: связанная гравитация и жизненный цикл процессов

Причина зависимости высоты прыжка от FPS при обычном `v -= g*h; x += v*h` —
неучтённое различие исходного kick-then-drift и частых коротких шагов.
Непрерывная модель использует `v'=a`, `x'=v+a/2` в единицах исходного тика;
без ограничителя она совпадает с исходной последовательностью на целых тиках.
При терминальной скорости L используются `v=max(L,v+a*h)` и аналитический
интеграл `max(L,v+a/2)`. Это сохраняет траекторию при разбиении шага и не
превышает нижний предел скорости перемещения. Возле момента достижения L
позиция может немного отличаться от дискретного оригинала; не заявляется
побитовое совпадение современного capped trajectory с оригинальным. Original
исполняет исходные инструкции без этой модели.

Правила гравитации: player801C0FA8(-55), actor801CB408(-50), mini8_801C42D0(-50).
Результат displacement живёт только в текущем SimulationScope, связан с адресом
velocity и её битовым значением после гравитации. Изменённая контактом/импульсом
скорость не получает старое displacement. Нет кэша указателей между сценами.
Используют его Y additions:801BF8A8/801BFDA4,801CB6EC,801E8E10,mini8_801C4318.
Геометрические отступы, размеры тела, шаги поиска стен8/20 и floor corrections
не масштабируются. Полный collision solver ещё не проверен этим компонентом.

native-gravity-check:786 сценариев на реальных трёх gravity и двух move функциях,
разные скорости/знаки ускорения/исходные положения относительно limit,
60/73/120/144/165/360FPS и20/30Hz; ASan/LSan/UBSan новых частей. Проверка
«контакта» здесь означает замену velocity между gravity и displacement,
а не прохождение настоящей геометрии уровня.

В общем dispatcher800281B8 статус1 уменьшает wait, статус4 становится2, и только
следующее исходное обновление вызывает активированный callback. Hook80028208
сохраняет эти переходы раз в исходный тик; активные callbacks вызываются на
каждом native draw. Особенно важно не вызвать новый status2 уже на следующем
дополнительном кадре того же тика. Учитывается обычный draw перед native draw.
В SimulationScope задержки продвигаются в начале тика; Original неизменён.
native-process-time-check:2053 случаев, все8 slots, статусы0..5, waits0/1/3,
сравнение с исходным dispatcher. Тест внутренней логики callbacks этим не заменён.

Общий native-loop-check также прошёл с новым dispatcher. Живой допуск сцен
ещё не расширен; эти исправления не завершают полный variable FPS.

## 2026-09-30: v82, обычная камера — незавершённый общий перевод

Разобрана таблица всех16 режимов dispatcher801DBD58 (15 уникальных handlers).
Modes0/1:DB7C8;2:D6EAC;3:D8FF0;4:D7294;5:D5864;6:D8578;7:DB720;
8:D9B00;9:D8A24;10:D9C94;11:DA06C;12:DAB58;13:DA588;14:DAF50;15:DB3D0.
Названия механик/уровней этим номерам ещё не присвоены. DB720 и D9B00 —
прямые геометрические камеры, временной фильтр к ним не добавлялся.

Общие формулы: approach1-(1-alpha)^h, retention^h. Для ограничения поворота
использован дробный iterate именно исходного e -> e-clamp(alpha*e,-L,L).
Порог T=L/alpha; k=-log(1-alpha). При |e|>T берётся n=ceil((|e|-T)/L),
phase=n+log((|e|-nL)/T)/k. Из phase-h обратно восстанавливается остаток ошибки.
При |e|<=T используется экспоненциальное приближение. На целых шагах получается
исходная последовательность даже при переходе через cap, а разбиение шага
композируется для неподвижной цели. Original проходит исходные инструкции.
Это вычисление уравнения, не RT64 interpolation готовых поз.

Фиксы первопричин:
- Continuation labels генератора должны стоять ДО остальных hooks того же PC.
  Иначе tick gate обходил соседнее scale движение (mode2 Y2 вместо1). v78+.
- В mode6 (D8578) Z проходила через signed16 временную переменную каждый кадр.
  Native дробные шаги теряли субединичное движение. Float сохраняется для
  fractional steps; целый шаг и Original сохраняют исходное округление.
- Коэффициент mode6 зависит от control+2 и ограничен speed(control+2+3);
  масштабировать только коэффициент недостаточно. Gain и cap обработаны вместе.
- Шесть recenter/obstruction timers идут по исходным тикам, отдельно от движения.
- Shake55EC: signed amplitude добавляется как displacement*h; decay и remaining
  меняются в конце исходного тика. Нет повторного полного импульса каждый draw.
- Две временные аннотации5EAC/78DC удалены: перед ними distance заново вычислен
  из геометрии глаза, это не обычный фильтр предыдущей distance. Пространственные
  шаги collision probes не масштабируются. Наличие умножения на0.1 не доказывает dt.

Проверки v82 без игры/GPU:970 control/transition/coefficient/shake cases с
реальными исходными функциями; Original/full-step RAM differential; 440 вариантов
capped approach по20 тиков;20/30Hz и60/73/120/144/165/360FPS. ASan/LSan,
UBSan нового C++. Отдельно все13 остальных roots:78 Original/full-tick RAM
сравнений. Synthetic actors, no-map path исходной collision query, не стены уровня.
Широкая проверка rate spread ДИАГНОСТИЧЕСКАЯ: остатки mode4/5 distance~1.28,
mode9 pitch~0.155deg, mode10 distance~1.60. Они не объявляются пройденными.
Mode6 spread~0.000023; остальные~0..0.000733 в этих сценариях.
Причины ещё исследуются: derived output против temporal state, задержка публикации
distance на один вызов, инициализация/смена collision pitch target, coupled filters.

Полная камера с препятствиями, внешний ввод/mode switch, feedback тряски через
фильтр, вся физика/player callbacks, 2D/миниигры и общий native update остаются
незавершёнными. Live policy всё ещё scene34 image/fade; v82 не означает нового
допуска сцены. Последний EXE test-029(v76). Эта запись — checkpoint, не завершение.

## v83: общий расчёт связанных уравнений ordinary camera

Оставшиеся v82 расхождения камер4/5/9/10 устранены в компонентной проверке
OrdinaryCameraTimeline. Исходный алгоритм последовательно вычисляет геометрию,
target filters и предыдущие distance/pitch targets. Повторять эту композицию
при каждом коротком шаге означало менять алгоритм. Теперь проверенные уравнения
вычисляются на накопленном времени текущего исходного тика с его начальным
состоянием. Цели/geometry берутся из текущего игрового состояния, готовые
previous/next poses не смешиваются. На границе1.0 получаем исходный результат.

Состояние ограничено конкретными полями каждого handler, включая halfword
границы; не копируется весь диапазон globals. По соседству есть чужие1823E,
18328/1832C и zoom halfword18278. Camera eye+0C/10/14 и up-vector+28 пишет
геометрический8002F064; они исключены из anchor. Внешняя запись в собственное
состояние камеры или смена handler/camera отключает anchor до следующей границы;
чужая запись не откатывается. reset очищает host bookkeeping без гостевых pointers.
Смена режима посреди тика пока использует incremental equations до новой границы.

Mode6 signed16 Z correction распределена по времени текущего тика, чтобы не
обрывать субединичное движение на каждом кадре и не получить скачок в его конце.
На полном шаге исходное округление сохраняется.

native-camera-modes-check v83:9571 сравнение/проверка,13 roots,6 synthetic states,
60/144/360FPS, исходные функции с реальной тригонометрией. На каждой20Hz границе
target/angles/distance совпадают с Original, на общих промежуточных timestamps
60/360 совпадает весь camera output. Есть проверка более30 изменений за секунду,
чтобы20Hz-only путь не мог пройти как native. Отдельно external writes/reset.

Для препятствий восстановлен grid format80040104: signed XZ bounds; relative
offsets polygons/lists/grid; cell50;44-byte triangles with projected XZ vertices,
normal(A,B,C), D for Ax+By+Cz=D, flags. Synthetic room uses actual original
query: floor0/ceiling250, inside flag0/outside flag1. Это не ROM level resources
и не прохождение реальных стен/уровней. Игровое окно/GPU не запускались.

Компонент добавлен в сборку, но LIVE scene policy НЕ расширена: общий scene
simulation update ещё требует соединения с ним и с physics/events. Полный FPS
остаётся незавершённым. CPU-проверка камеры не является проверкой всего game loop.


## Блок5: общий2D/UI, generation355 — реализация перед проверкой

UI frame scope подключён к существующему game loop для scene1 gameplay/cutscene,
image и четырёх пар menu callbacks overlay2 (title/files/intro/selection). При
неподключённой мировой подсистеме отдельная UI policy строит Native кадры;
мировые Source callbacks продолжают исходные20/30Hz, их готовность не заявляется.
Восьми panel records добавлено дробное вычисление исходного quantized-rate
уравнения; screen rectangle — четыре float bounds с исходным source counter.
Четыре режима title letters выполняют actual guest math в isolated4MiB image:
kick/drift/cap, slide, exit/scale и alternate bounce; публикуются только visible
floats. Intro alpha, fixed selection actor sin bob/rotation, text row movement
пересчитываются по времени. UI не использует RT64 pose interpolation.

Фактический widgets draw продвигает reveal/lifetime при phase1; Native сохраняет
один шаг на Source epoch, дополнительные draws восстанавливают только widget
records. Dialogue arrow, files visual counters и selection panel growth тоже
сохраняют исходную частоту. Исходное число text-draw sound requests внутри первого Source draw сохранено; дополнительные кадры их не повторяют. Button/repeat,
dialogue/page/answer/busy, saves/pak/EEPROM, transitions, waits/finish flags
остаются Original. Временные visible fields восстановлены до следующего Source
update; callback pairs требуют правильного loaded overlay, divider2/3 и
безопасной framebuffer dependency. Старые player/NPC/dialogue fixes сохранены.

Точные ROM sites/body identities, понятные имена/роли/комментарии и документы
обновлены. Следующая проверка — только связанный UI блок перед Windows040;
неизменённые player/NPC/weapon/pickup/world suites не запускать. Игру/GPU не
запускать. Реальный вид меню, текстур/шрифтов и прохождение проверяет пользователь.
Полный variable FPS всей игры ещё требует блоков1/2/4/6. По завершении5 предложить
эти блоки на выбор, следующий самостоятельно не начинать.

Source dispatcher отмечает фактически вызванные UI domains. Пауза или ветвь,
пропустившая panels/rectangle/title/intro/text math, не получает дробного шага.
При повторном draw берётся состояние перед соответствующим Original вызовом,
а после него восстанавливается уже сохранённый Source результат. Это сохраняет
не только счётчики, но и тот же видимый text reveal; несколько views сохраняют
исходное число шагов. UI clock использует собственное Source wall-time.

Полный разбор63B8 уточнил роль: это arrow texture builder с blink и sound,
а не только звук. Дополнительные Native кадры повторяют geometry/commands;
только JAL444C0 пропускается, blink/sound flags получают Original pre-draw
состояние и затем Source результат восстанавливается. Каталог/документы исправлены.


## Блок5 завершён — main355 / test-040

Windows040 сохранён: `D:/Games/DoraRecomp3/testbuild/Dora64-3-test-040.exe`.
SHA256 `78362e29bdaf6f7166f1d13cf50c4c9b3cb813879c80f1054fdf8349e02c9cce`. Общий2D/UI подключён к Native Manual: новые display lists
в title/files/intro/selection и scene1 UI для gameplay/cutscene/pause branches.
Панели считают start+quantized Source rate*(elapsed+h), screen rectangle —
четыре float bounds, title letters — четыре actual guest equation режима,
intro — alpha, selection — fixed actor sin bob/rotation и text row movement.
RT64 pose interpolation не используется. При неподключённой мировой подсистеме
UI policy оставляет world Source callbacks20/30Hz: это не заявляет Native мир.

Resident dispatcher наблюдает фактически вызванные Source UI domains; пропущенная
ветвь или пауза не продолжают math. World/script/camera/resource updates не
reinvoкe на дополнительных UI кадрах. Контроллер/repeat/answers/pages/commands,
waits/completion, saves/pak/EEPROM и text token cursors сохраняют Original порядок.
Widgets reveal/lifetime находились в phase1 draw; после первого Native draw
запоминается результат Source epoch. Дополнительный draw получает pre-state того
же Original вызова и восстанавливает post-state после построения команд. Это
сохраняет glyph reveal и Original multiplicity нескольких views. Dialogue bob,
files visual counters, text arrow blink/sound flags и final panel growth защищены
тем же владельцем.63B8 оказался полноценным arrow texture builder: geometry
исполняется каждый кадр, подавляется только повторный sound JAL444C0. UI scope
восстанавливает math fields/context до следующего Source input и reset/handoff.

Fresh targeted355:80 connected UI cases,64 Original motion endpoints,
153 Native math samples,72 repeated Original draw/command comparisons и37
ownership/pause/policy guards; Original20/30, Manual60/73, full math RAM/context restore,
ASan/UBSan. actual UI/math bodies берутся из Original ROM, reference61 не имеет
Native hooks в выбранном замыкании. Texture-info и textured glyph/rectangle/string
draw, sound request — counted fixture boundaries; реальные ресурсы/GPU/вид меню
не проверялись. Resident dispatcher/policy и UI hook sites закреплены42 точными
ROM инструкциями; unrelated world/player/NPC/weapon/pickup/camera suites не запускались.

39 разобранных UI функций получили понятные имена и описания,9 точных областей
переменных добавлены. Всего849 descriptions/518 semantic aliases/596 register
scopes. Names355:96 C units, exact canonical round trip/preprocessed tokens,
C17 syntax compilation, прежние ABI/ROM identity/register storage. Документы,
address map и соответствия старым функциям актуальны. Историческое pending в
catalog evidence фиксирует подготовку; результат подтверждён current355 proofs.

Windows MSVC RelWithDebInfo собран; EXE совпадает с checked source/generation.
32 dependency pins/donor неизменны. Source354/test039 архивирован перед040;
старые EXE/DLL/ROM/settings/saves сохранены. Локального запуска игры/GPU не было.
Первая CPU compile выявила требования RDRAM macro identifier и unused helper:
исправлены только новые UI helper functions, затем повторена лишь UI проверка
с готовым C object cache; уже прошедший names check не повторялся. Первая Windows
compile получила C1902 в служебном PDB manager ограниченного процесса; повтор
сборки в доступном окружении отдельно отражён в локальных логах. Windows
compile также выявила отсутствующий namespace двух UI policy helpers: исправлена
только их регистрация, затем та же focused fixture включает actual policy
selection всех пяти callback pairs20/30 и framebuffer rejection.


Блок5 закрыт на уровне общей UI реализации и целевой CPU/Windows сборки. Полный
variable FPS всей игры ещё не завершён. Следующий блок выбирает пользователь:
1 объекты/платформы;2 враги/активный AI;4 миниигры/специальные режимы;6 общая
интеграция. Другой блок не начинать без выбора.
