# Manual FPS: статический проход148

Проход по перечисленным типам завершён перед сборкой. Проверка выполнена по актуальным исходникам, сгенерированному представлению Original и ROM. Игра, эмулятор, CPU checks, тесты и GPU не запускались. Список выдан пользователю перед компиляцией. После него выполнена Windows Release148 без журналов; приложение не запускалось.

Индекс:218 enemy-записей,292 scene-записи и29 записей отдельных NPC/эффектов;539 сочетаний типа/банка,536 уникальных числовых типов. Включены варианты и технические пустые записи. Это размер индекса, не число самостоятельных врагов.

## Что проверено

- Source кадры и end/clamp/wrap/reverse; собственный период клипа, повтор cue, bind/seek/pause и группы моделей.
- Вход, контакт, ожидания, переходы, счётчики, RNG, spawn/unlink/retire; одно выполнение Source FSM и исходный порядок пула.
- Проекции, scalar/heading/pitch, gravity/kick-before-drift, реальные floor/ceiling/wall/route/arena setters.
- Существующие linear/approach/exponential/sine/scale/opacity расчёты и фактический порядок публикации alpha.
- Родитель, части, дополнительные группы, исторические очереди и тени; slot/lifetime/run, предшествующий Source sample и геометрия без повторных команд.

## Найдено и исправлено

1. Видимость bit4 ошибочно участвовала в проверках идентичности часов анимации. Hide/show мог начать новую дробную сетку. Теперь часы сохраняются; восстановление снимков сохраняет актуальную видимость, end/clamp/pause проверяются отдельно.
2. При переходе Manual → Original использовался старый Source sample FSM; при возврате Original → Manual могла восстанавливаться показанная дробная поза вместо удержанной Source-сетки. Сохранён текущий Source sample и его baseline для обратного перехода. Например, потеря сетки0/3/6 приводила к32.4/35.4 вместо достижимого33.
3. Основной enemy/scene обработчик защищал часы только group0. Теперь подключены все восемь зарегистрированных групп; городские и slot09 NPC с отдельным диспетчером также сохраняют локальные часы и Source end sample.
4. Четыре точных события не входили в Source scope адаптера: Rikurage2 frame27/spawn Light338; ControlStone529 frame60; MetarudoDoor530/531 frame0/pause-return; общий801EB670 frame87/reward. Добавлен Source путь и защита повторов, исходное точное равенство сохранено.
5. Включены уже собранные147 правки: общий clip-local sampling/dedup; четыре перепутанных scale-offset Boss4 FireBall442/Crash445; абсолютный индекс Brobo collision rewind. Номера440/444 в прежней заметке147 были ошибочными подписями объектов.

Две подозрительные записи Oppi/Powan оказались ложными срабатываниями линейных имён регистров: проверены реальные ветви MIPS, правок этих записей нет.619 строк Store/SceneStore совпадают с ROM по offset и FPR; четыре SW(a2) правильно получают float bits через MFC1. Совпадение таблиц само по себе не доказывает правильность всей траектории.

## Перечень семейств и связанных объектов

- **.demo_slot0**: Pyonger1/2, снаряд; Barupu1/2, нота; Brobo A100/A101/A103/A104, два снаряда и эффект. Типы: 266, 267, 268, 341, 342, 343, 344, 345, 346, 347, 297, 298, 299.
- **.demo_slot1**: Kinopper1/2; Bironn и сегменты; Oppi1/2 и подарки; три варианта бабочки. Типы: 264, 265, 272, 273, 316, 317, 374, 375, 376.
- **.remaining_slot0_03**: Doridori; BroboDC08 Fake/Real/T2/T3, бомба и Hob. Типы: 320, 348, 349, 350, 351, 352, 353.
- **.remaining_slot0_04**: Sabotti1/2/3; Baguram1/2/3 и генератор; Gokireon1/2. Типы: 269, 270, 271, 277, 278, 279, 280, 281, 282.
- **.remaining_slot0_05**: Batti1/2 и Honeycomb; Thunder1/2, генератор и разряд; Tamabonga1/2 и две бомбы. Типы: 287, 288, 289, 283, 284, 285, 286, 293, 294, 295, 296.
- **.remaining_slot1_01**: Arizinogu1/2 и снаряд; Tolneder; сюжетная дверь. Типы: 321, 290, 291, 292, 579.
- **.remaining_slot1_02**: Doragonrock1/2 и огонь; KaenDan/KaenHousha, генераторы и сегменты огня; огненная ловушка, опора и двери. Типы: 274, 275, 276, 370, 371, 372, 373, 511, 538, 539, 580, 581.
- **.remaining_slot1_06**: Metabo; Gabugon и язык; Kirafu, дым и частица. Типы: 322, 323, 324, 395, 396, 397.
- **.cave_slot0**: Supaizan1/2, паутина и снаряд. Типы: 312, 313, 314, 315.
- **.cave_slot1**: Derorin1/2/3; Churatto и Banyatto. Типы: 308, 309, 310, 318, 319.
- **.mountain_slot1**: Tutaru; TwinFace, огонь и луна; две двери. Типы: 326, 327, 328, 329, 493, 494.
- **.lake_slot0**: Fisher1/2; Yadokaringer1/2 и два пузыря; Rikurage1/2 и свет; Tatsunoorgo и пузырь. Типы: 339, 340, 330, 331, 332, 333, 334, 335, 336, 337, 338.
- **.lake_slot1**: IgaHead и большая сценовая голова; Sentoutei1/2/3 и снаряд; Houdai, основание и снаряд; Majin и дух; Iwa и JyaiCong; дверь. Типы: 325, 392, 382, 383, 384, 385, 386, 398, 366, 367, 368, 369, 512, 584.
- **.lake_slot2**: Sakana1/2; Soundorago1/2, сегменты, хвосты, всплеск и холм; Posedo; BroboNB0 NPC; двери. Типы: 377, 378, 393, 300, 301, 302, 303, 304, 305, 306, 307, 78, 495, 578.
- **.level1_slot0**: MiniCar1/2 и дым; PatPot и снаряд; Powan1/2/3/4 и эффект превращения; падающий Derorin; феи75/76. Типы: 359, 360, 361, 362, 363, 387, 388, 389, 390, 391, 311, 75, 76.
- **.remaining_slot1_04**: RadiconRobo/Car; Thunder, предупреждение и искры; KaitenDai, столб, кнопка и стекло; ControlStone; обе двери и панели Metarudo; TowerFlash. Типы: 379, 380, 381, 364, 365, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534.
- **.remaining_slot0_11**: Boss1: интро, выбор атак, молния, волны, вихри, водяные столбы, смерть; Hole, Shield, Lightning, WaterPole, Splash, Vortex/Trail, Tsunami, Crash, FirePot. Типы: 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409.
- **.remaining_slot0_12**: Boss2: все состояния головы, body/leg1–4/10–12, ghost100; Spark, Rocket, ArmFlash, оба дыма, Bomb/Shot, Laser/Effect, Missile, Fannel/Laser, Fire, BackFlash, Crash. Типы: 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427.
- **.remaining_slot0_13**: Boss3: все10 состояний тела; Shot, SoundWave, Pipe1/2/3/10, Apple, Plate1/2/3, Crash. Типы: 428, 429, 430, 431, 432, 433, 434.
- **.remaining_slot0_14**: Boss4: все10 состояний тела; Suzu, Hitode, Doka, RoomCtrl, TimeHole/Machine, FireBall442, Shot, Smoke, Crash445. Типы: 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445.
- **.remaining_slot0_15**: Boss5: интро, выбор/полёт, лазер, рывок4, flare5, shield6/reverse, атака7 и смерть; Slime, XBeam, Laser/Flare, Shield, Shadow, Crash. Типы: 446, 447, 448, 449, 450, 451, 452, 453.
- **.remaining_slot0_16**: Boss6: все состояния головы и ghost100, body/leg1–4/10–12; Spark, Rocket, ArmFlash, оба дыма, Bomb/Shot, Laser/Effect, Missile, Fannel/Laser, Fire, BackFlash, Crash. Типы: 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471.
- **.remaining_slot0_17**: Boss7: intro1, пустые2/5, homing3, mask-loss4, jump6 и смерть10; Mask, Slime, Shield, Crash, Wall. Типы: 472, 473, 474, 475, 476, 477.
- **.penguin_slot1**: Pepin stay/walk/slime; Minikku stay/walk; Popika stay/walk/slime/fly/fuyuu; Doragonia stay/guard/boss, все варианты. Типы: 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 68, 69, 70.
- **.penguin_slot2**: Doragonia1/2; ветряк, свет, монорельс, шестерня и платформа. Типы: 66, 67, 505, 506, 507, 565, 566.
- **.remaining_slot1_09**: Dakuto stay/walk, мальчики/девочки и keeper; NPC24–28, обычные и сюжетные100/101/102/200 состояния. Типы: 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 24, 25, 26, 27, 28.
- **.level1_slot1**: Городские7/9; мамы Нобиты/Сунео/Шизуки/Джайана, Jaiko, Sensei, Kaminari, Dekisugi; parent18/child19 и слизь6, профили0/1/2. Типы: 7, 9, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22.
- **.scene1**: Общие Explosion1/2/Reflection, огонь/холод/разряд урона, исчезновение; двери/warp/сундуки/контейнер, свет/стрелка/всплеск; NPC71–74/80; эффекты129–131; пустые записи и служебные TestChr. Типы: 394, 257, 258, 259, 109, 110, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 260, 261, 262, 263, 481, 482, 483, 484, 485, 486, 487, 518, 535, 536, 544, 547, 586, 589, 591, 6, 71, 72, 73, 74, 80, 129, 130, 131.
- **.cave_slot2**: T00 Shield controller/effect/completed; Domero/Ratena переключатели и плита; двери, щит, Majin light/effect. Типы: 354, 355, 356, 489, 497, 498, 499, 500, 559, 560, 561, 582, 583.
- **.demo_slot2**: T17 Door controller/completed; Sprit79, Mariru plate/switch, DancingFlower; Crystal, Spring, Wheel1/2/Gondola, bridge, Dezu switch/plate. Типы: 357, 358, 79, 490, 491, 492, 496, 513, 519, 562, 563, 564, 568, 574, 575, 576.
- **.remaining_slot2_08**: Sekiban81, paint block/bridge, sound switch/door/три света/tama, StoneArm/Speaker, опора и Goddess. Типы: 81, 479, 480, 488, 514, 515, 516, 517, 537, 540, 542, 567, 588.
- **.remaining_slot2_04**: Все варианты дверей M009/M015, движение и клипы видимости. Типы: 501, 502, 503, 504, 508, 509, 510.
- **.mountain_slot2**: Kumo, Jack стебель/лист/земля, Metarudo shield controller, Dorami time machine. Типы: 520, 522, 523, 524, 577, 585.
- **.remaining_slot2_07**: Taiyo, WaterDrop, большие/малые IcePlate, Snow, Kogooru switch/plate. Типы: 521, 541, 545, 548, 549, 556, 557, 558.
- **.remaining_slot2_09**: Miire платформы101–104/107, switch/control; Baguru switches. Типы: 550, 551, 552, 553, 554, 555, 569, 570, 571, 572, 573.
- **.level1_slot2**: BroboA100Break, Taiyo/WaterDrop/Book, Saku, NobiheyaSlime/Drawer; кот77. Типы: 478, 521, 541, 543, 545, 546, 587, 590, 77.

## Boss5 и пределы вывода

На скриншоте Boss5 kind446/model9A5/bank10DA30. Проверены constructor/contact и42 функции root/linked closure. Clip14 длится48, cue33 начинает рывок; дальше clip16/end и clip23/end. Разобраны полёт, лазер, flare, отдельный shield/reverse42/0, shadow и смерть. Исходный rate2 в атаке7 сохранён. Таймауты и принудительная смена фаз не добавлялись.

Потеря сетки часов доказана статически и исправлена. Что именно вызвало показанное пользователем зависание, без игрового состояния окончательно не установлено. Этот билд должен проверить пользователь; компиляция не будет заявляться подтверждением исчезновения зависания.

Рост/затухание огня TwinFace328, Doragonrock276, KaenDan371, KaenHousha373 и TrapFire539; общий DamageFire260; FirePot409; FireBall442; Fire425/469, дым и эффекты боссов включены в проход. Новых подтверждённых дефектов в их уравнениях после прежних исправлений не обнаружено.

Ранее подтверждённое владельцем поведение Sentoutei в Original ROM сохраняется. Старые отметки synthetic/connected CPU в описаниях Source-функций не включены в валидацию148.

## Поштучный индекс

| Тип | Банк | Callback | Роль | Проверка |
|---:|---|---|---|---|
| 6 | .scene1 | dora3_object_kind6_update_801EB864 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 7 | .level1_slot1 | dora3_city_kind7_state_dispatch_8022D938 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 9 | .level1_slot1 | dora3_city_kind9_state_dispatch_8022DBB0 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 13 | .level1_slot1 | dora3_nobimama_state_dispatch_and_walk_velocity_8022E480 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 14 | .level1_slot1 | dora3_sunemama_state_dispatch_and_walk_velocity_8022ED78 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 15 | .level1_slot1 | dora3_sizumama_state_dispatch_and_walk_velocity_8022F648 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 16 | .level1_slot1 | dora3_jaimama_state_dispatch_and_walk_velocity_8022FF44 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 17 | .level1_slot1 | dora3_jaiko_state_dispatch_and_walk_velocity_80230830 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 18 | .level1_slot1 | dora3_city_kind18_dispatch_and_publish_linked_clip_80230F70 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 19 | .level1_slot1 | dora3_city_kind19_state1_sync_parent_clip_8023098C | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 20 | .level1_slot1 | dora3_sensei_state_dispatch_and_walk_velocity_802317CC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 21 | .level1_slot1 | dora3_kaminari_state_dispatch_and_walk_velocity_802322EC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 22 | .level1_slot1 | dora3_dekisugi_state_dispatch_and_walk_velocity_80232BD0 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 24 | .remaining_slot1_09 | dora3_npc_slot09_kind24_dispatch_states_8022DFA4 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 25 | .remaining_slot1_09 | dora3_npc_slot09_kind25_dispatch_states_8022EB94 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 26 | .remaining_slot1_09 | dora3_npc_slot09_kind26_dispatch_states_8022F674 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 27 | .remaining_slot1_09 | dora3_npc_slot09_kind27_dispatch_states_80230448 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 28 | .remaining_slot1_09 | dora3_npc_slot09_kind28_dispatch_states_8022FE4C | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 29 | .penguin_slot1 | dora3_npc_pepin_stay_update_8022DE28 | scene | Source states/Native categories; игровая проверка ожидается |
| 30 | .penguin_slot1 | dora3_npc_pepin_stay_update_8022DE28 | scene | Source states/Native categories; игровая проверка ожидается |
| 31 | .penguin_slot1 | dora3_npc_pepin_walk_update_8022E7B4 | scene | Source states/Native categories; игровая проверка ожидается |
| 32 | .penguin_slot1 | dora3_npc_pepin_walk_update_8022E7B4 | scene | Source states/Native categories; игровая проверка ожидается |
| 33 | .penguin_slot1 | dora3_npc_pepin_slime_update_8022E9A8 | scene | Source states/Native categories; игровая проверка ожидается |
| 34 | .penguin_slot1 | dora3_npc_pepin_slime_update_8022E9A8 | scene | Source states/Native categories; игровая проверка ожидается |
| 35 | .penguin_slot1 | dora3_npc_minikku_stay_update_8022F2C8 | scene | Source states/Native categories; игровая проверка ожидается |
| 36 | .penguin_slot1 | dora3_npc_minikku_stay_update_8022F2C8 | scene | Source states/Native categories; игровая проверка ожидается |
| 37 | .penguin_slot1 | dora3_npc_minikku_walk_update_8022FC90 | scene | Source states/Native categories; игровая проверка ожидается |
| 38 | .penguin_slot1 | dora3_npc_minikku_walk_update_8022FC90 | scene | Source states/Native categories; игровая проверка ожидается |
| 39 | .penguin_slot1 | dora3_npc_popika_stay_update_80230494 | scene | Source states/Native categories; игровая проверка ожидается |
| 40 | .penguin_slot1 | dora3_npc_popika_stay_update_80230494 | scene | Source states/Native categories; игровая проверка ожидается |
| 41 | .penguin_slot1 | dora3_npc_popika_walk_update_80230B48 | scene | Source states/Native categories; игровая проверка ожидается |
| 42 | .penguin_slot1 | dora3_npc_popika_slime_update_80230D5C | scene | Source states/Native categories; игровая проверка ожидается |
| 43 | .penguin_slot1 | dora3_npc_popika_slime_update_80230D5C | scene | Source states/Native categories; игровая проверка ожидается |
| 44 | .penguin_slot1 | dora3_npc_popika_fly_update_80230F48 | scene | Source states/Native categories; игровая проверка ожидается |
| 45 | .penguin_slot1 | dora3_npc_popika_fly_update_80230F48 | scene | Source states/Native categories; игровая проверка ожидается |
| 46 | .penguin_slot1 | dora3_npc_popika_fuyuu0_update_80231A90 | scene | Source states/Native categories; игровая проверка ожидается |
| 47 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 48 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 49 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 50 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 51 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 52 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 53 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 54 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 55 | .remaining_slot1_09 | dora3_npc_dakuto_stay_update_802311F8 | scene | Source states/Native categories; игровая проверка ожидается |
| 56 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 57 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 58 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 59 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 60 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 61 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 62 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 63 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 64 | .remaining_slot1_09 | dora3_npc_dakuto_walk_update_802320D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 65 | .remaining_slot1_09 | dora3_npc_dakuto_keeper_update_80232910 | scene | Source states/Native categories; игровая проверка ожидается |
| 66 | .penguin_slot2 | dora3_npc_doragonia1_update_8023E7D0 | scene | Source states/Native categories; игровая проверка ожидается |
| 67 | .penguin_slot2 | dora3_npc_doragonia2_update_8023EC14 | scene | Source states/Native categories; игровая проверка ожидается |
| 68 | .penguin_slot1 | dora3_npc_doragonia_stay0_update_80232238 | scene | Source states/Native categories; игровая проверка ожидается |
| 69 | .penguin_slot1 | dora3_npc_doragonia_guard_update_80232690 | scene | Source states/Native categories; игровая проверка ожидается |
| 70 | .penguin_slot1 | dora3_npc_doragonia_boss_update_80232C04 | scene | Source states/Native categories; игровая проверка ожидается |
| 71 | .scene1 | dora3_orbit_object_kind71_74_update_801EEFC8 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 72 | .scene1 | dora3_orbit_object_kind71_74_update_801EEFC8 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 73 | .scene1 | dora3_orbit_object_kind71_74_update_801EEFC8 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 74 | .scene1 | dora3_orbit_object_kind71_74_update_801EEFC8 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 75 | .level1_slot0 | dora3_fairy_state_dispatch_and_vertical_bob_80223FBC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 76 | .level1_slot0 | dora3_fairy_state_dispatch_and_vertical_bob_80223FBC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 77 | .level1_slot2 | dora3_cat_state_dispatch_8023DF3C | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 78 | .lake_slot2 | dora3_npc_brobo_n_b0_update_802401A0 | scene | Source states/Native categories; игровая проверка ожидается |
| 79 | .demo_slot2 | dora3_scene_object_sprit_update_8023DCE4 | scene | Source states/Native categories; игровая проверка ожидается |
| 80 | .scene1 | dora3_object_kind80_noop_801EF2EC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 81 | .remaining_slot2_08 | dora3_scene_object_sekiban_update_8023F730 | scene | Source states/Native categories; игровая проверка ожидается |
| 109 | .scene1 | dora3_scene_object_dokodemodoa_update_801EA86C | scene | Source states/Native categories; игровая проверка ожидается |
| 110 | .scene1 | dora3_scene_object_dokodemodoalight_update_801EA4D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 129 | .scene1 | dora3_pickup_linked_effect_update_source_801ED2CC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 130 | .scene1 | dora3_pickup_flash_dispatch_delay_motion_801ED898 | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 131 | .scene1 | dora3_periodic_pickup_effect_fall_and_retire_801ED8FC | legacy_npc_or_effect | Source states/Native categories; игровая проверка ожидается |
| 132 | .scene1 | dora3_scene_object_testchr1_update_801E9ECC | scene | Source states/Native categories; игровая проверка ожидается |
| 133 | .scene1 | dora3_scene_object_testchr2_update_801E98E0 | scene | Source states/Native categories; игровая проверка ожидается |
| 134 | .scene1 | dora3_scene_object_testchr3_update_801E9664 | scene | Source states/Native categories; игровая проверка ожидается |
| 135 | .scene1 | dora3_scene_object_testchr4_update_801E9428 | scene | Source states/Native categories; игровая проверка ожидается |
| 136 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 137 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 138 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 139 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 140 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 141 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 142 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 143 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 144 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 145 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 146 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 147 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 148 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 149 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 150 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 151 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 152 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 153 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 154 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 155 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 156 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 157 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 158 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 159 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 160 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 161 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 162 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 163 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 164 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 165 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 166 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 167 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 168 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 169 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 170 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 171 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 172 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 173 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 174 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 175 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 176 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 177 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 178 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 179 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 180 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 181 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 182 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 183 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 184 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 185 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 186 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 187 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 188 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 189 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 190 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 191 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 192 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 193 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 194 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 195 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 196 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 197 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 198 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 199 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 200 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 201 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 202 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 203 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 204 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 205 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 206 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 207 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 208 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 209 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 210 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 211 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 212 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 213 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 214 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 215 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 216 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 217 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 218 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 219 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 220 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 221 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 222 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 223 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 224 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 225 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 226 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 227 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 228 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 229 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 230 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 231 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 232 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 233 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 234 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 235 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 236 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 237 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 238 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 239 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 240 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 241 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 242 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 243 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 244 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 245 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 246 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 247 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 248 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 249 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 250 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 251 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 252 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 253 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 254 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 255 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 256 | .scene1 | dora3_scene_object_shoumetsu0_update_801E52D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 257 | .scene1 | dora3_enemy_explosion_primary_update_801E53D0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 258 | .scene1 | dora3_enemy_explosion_secondary_update_801E5598 | enemy | Source states/Native categories; игровая проверка ожидается |
| 259 | .scene1 | dora3_enemy_reflection_effect_update_801E57C0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 260 | .scene1 | dora3_scene_object_efdamagefire_update_801E58E0 | scene | Source states/Native categories; игровая проверка ожидается |
| 261 | .scene1 | dora3_scene_object_efdamagecold_update_801E5A94 | scene | Source states/Native categories; игровая проверка ожидается |
| 262 | .scene1 | dora3_scene_object_efdamagekanden_update_801E5C2C | scene | Source states/Native categories; игровая проверка ожидается |
| 263 | .scene1 | dora3_scene_object_huyuplate_update_801EA000 | scene | Source states/Native categories; игровая проверка ожидается |
| 264 | .demo_slot1 | dora3_kinopper_t1_update_8022DC90 | enemy | Source states/Native categories; игровая проверка ожидается |
| 265 | .demo_slot1 | dora3_kinopper_t2_update_8022E52C | enemy | Source states/Native categories; игровая проверка ожидается |
| 266 | .demo_slot0 | dora3_pyonger1_update_8021FAE4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 267 | .demo_slot0 | dora3_pyonger2_update_80220830 | enemy | Source states/Native categories; игровая проверка ожидается |
| 268 | .demo_slot0 | dora3_pyonger_projectile_update_80220CD0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 269 | .remaining_slot0_04 | dora3_sabotti1_update_80220358 | enemy | Source states/Native categories; игровая проверка ожидается |
| 270 | .remaining_slot0_04 | dora3_sabotti2_update_80220BAC | enemy | Source states/Native categories; игровая проверка ожидается |
| 271 | .remaining_slot0_04 | dora3_sabotti3_update_8022164C | enemy | Source states/Native categories; игровая проверка ожидается |
| 272 | .demo_slot1 | dora3_bironn_update_8022F0EC | enemy | Source states/Native categories; игровая проверка ожидается |
| 273 | .demo_slot1 | dora3_bironn_segment_update_8022F56C | enemy | Source states/Native categories; игровая проверка ожидается |
| 274 | .remaining_slot1_02 | dora3_doragonrock_update_8022DE08 | enemy | Source states/Native categories; игровая проверка ожидается |
| 275 | .remaining_slot1_02 | dora3_doragonrock2_update_8022F314 | enemy | Source states/Native categories; игровая проверка ожидается |
| 276 | .remaining_slot1_02 | dora3_doragonrock_fire_update_8022F67C | enemy | Source states/Native categories; игровая проверка ожидается |
| 277 | .remaining_slot0_04 | dora3_baguram_update_8021E288 | enemy | Source states/Native categories; игровая проверка ожидается |
| 278 | .remaining_slot0_04 | dora3_baguram2_update_8021F444 | enemy | Source states/Native categories; игровая проверка ожидается |
| 279 | .remaining_slot0_04 | dora3_baguram3_update_8021F77C | enemy | Source states/Native categories; игровая проверка ожидается |
| 280 | .remaining_slot0_04 | dora3_baguram_generator_update_8021FBA4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 281 | .remaining_slot0_04 | dora3_gokireon1_update_80222458 | enemy | Source states/Native categories; игровая проверка ожидается |
| 282 | .remaining_slot0_04 | dora3_gokireon2_update_802236D0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 283 | .remaining_slot0_05 | dora3_thunder1_update_80220DF4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 284 | .remaining_slot0_05 | dora3_thunder2_update_802213E4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 285 | .remaining_slot0_05 | dora3_thunder_generator_update_80221740 | enemy | Source states/Native categories; игровая проверка ожидается |
| 286 | .remaining_slot0_05 | dora3_thunder_discharge_update_802218BC | enemy | Source states/Native categories; игровая проверка ожидается |
| 287 | .remaining_slot0_05 | dora3_batti_update_8021E8C4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 288 | .remaining_slot0_05 | dora3_honeycomb_update_8021F070 | enemy | Source states/Native categories; игровая проверка ожидается |
| 289 | .remaining_slot0_05 | dora3_batti2_update_80220190 | enemy | Source states/Native categories; игровая проверка ожидается |
| 290 | .remaining_slot1_01 | dora3_arizinogu1_update_8022DB0C | enemy | Source states/Native categories; игровая проверка ожидается |
| 291 | .remaining_slot1_01 | dora3_arizinogu2_update_8022E94C | enemy | Source states/Native categories; игровая проверка ожидается |
| 292 | .remaining_slot1_01 | dora3_arizinogu_projectile_update_8022EE20 | enemy | Source states/Native categories; игровая проверка ожидается |
| 293 | .remaining_slot0_05 | dora3_tamabonga1_update_8022219C | enemy | Source states/Native categories; игровая проверка ожидается |
| 294 | .remaining_slot0_05 | dora3_tamabonga_bomb_animate_and_retire_80222AB4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 295 | .remaining_slot0_05 | dora3_tamabonga2_update_80222A2C | enemy | Source states/Native categories; игровая проверка ожидается |
| 296 | .remaining_slot0_05 | dora3_tamabonga_large_bomb_expand_spin_and_fade_80222B68 | enemy | Source states/Native categories; игровая проверка ожидается |
| 297 | .demo_slot0 | dora3_barupu_update_80221898 | enemy | Source states/Native categories; игровая проверка ожидается |
| 298 | .demo_slot0 | dora3_barupu2_update_802223E4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 299 | .demo_slot0 | dora3_barupu_note_update_80222A74 | enemy | Source states/Native categories; игровая проверка ожидается |
| 300 | .lake_slot2 | dora3_soundorago1_update_and_store_history_8023E220 | enemy | Source states/Native categories; игровая проверка ожидается |
| 301 | .lake_slot2 | dora3_soundorago_part_update_8023EFB0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 302 | .lake_slot2 | dora3_soundorago_part_update_8023EFB0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 303 | .lake_slot2 | dora3_soundorago_ripple_ground_frame36_spawn_and_retire_8023E438 | enemy | Source states/Native categories; игровая проверка ожидается |
| 304 | .lake_slot2 | dora3_soundorago_mound_bind_ground_clip_and_retire_8023E32C | enemy | Source states/Native categories; игровая проверка ожидается |
| 305 | .lake_slot2 | dora3_soundorago2_update_8023ECF8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 306 | .lake_slot2 | dora3_soundorago_part_update_8023EFB0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 307 | .lake_slot2 | dora3_soundorago_part_update_8023EFB0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 308 | .cave_slot1 | dora3_derorin_update_8022F598 | enemy | Source states/Native categories; игровая проверка ожидается |
| 309 | .cave_slot1 | dora3_derorin2_update_8022FB84 | enemy | Source states/Native categories; игровая проверка ожидается |
| 310 | .cave_slot1 | dora3_derorin3_update_80230310 | enemy | Source states/Native categories; игровая проверка ожидается |
| 311 | .level1_slot0 | dora3_derorin_falling_effect_update_8021F67C | enemy | Source states/Native categories; игровая проверка ожидается |
| 312 | .cave_slot0 | dora3_supaizan_update_8021E620 | enemy | Source states/Native categories; игровая проверка ожидается |
| 313 | .cave_slot0 | dora3_supaizan_web_update_8021E850 | enemy | Source states/Native categories; игровая проверка ожидается |
| 314 | .cave_slot0 | dora3_supaizan2_update_8021F39C | enemy | Source states/Native categories; игровая проверка ожидается |
| 315 | .cave_slot0 | dora3_supaizan_shot_update_8021F804 | enemy | Source states/Native categories; игровая проверка ожидается |
| 316 | .demo_slot1 | dora3_oppi_update_8022FF00 | enemy | Source states/Native categories; игровая проверка ожидается |
| 317 | .demo_slot1 | dora3_oppi2_update_80230384 | enemy | Source states/Native categories; игровая проверка ожидается |
| 318 | .cave_slot1 | dora3_churatto_update_8022DE70 | enemy | Source states/Native categories; игровая проверка ожидается |
| 319 | .cave_slot1 | dora3_banyatto_update_8022EA84 | enemy | Source states/Native categories; игровая проверка ожидается |
| 320 | .remaining_slot0_03 | dora3_doridori_update_802210A8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 321 | .remaining_slot1_01 | dora3_tolneder_update_8022F3F4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 322 | .remaining_slot1_06 | dora3_metabo_update_8022DC10 | enemy | Source states/Native categories; игровая проверка ожидается |
| 323 | .remaining_slot1_06 | dora3_gabugon_update_8022E4B4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 324 | .remaining_slot1_06 | dora3_gabugon_tongue_update_8022E7C0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 325 | .lake_slot1 | dora3_igahead_update_8022D958 | enemy | Source states/Native categories; игровая проверка ожидается |
| 326 | .mountain_slot1 | dora3_tutaru_update_8022DA28 | enemy | Source states/Native categories; игровая проверка ожидается |
| 327 | .mountain_slot1 | dora3_twinface_update_8022E4C8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 328 | .mountain_slot1 | dora3_twinface_fire_update_8022E81C | enemy | Source states/Native categories; игровая проверка ожидается |
| 329 | .mountain_slot1 | dora3_twinface_moon_update_8022EC34 | enemy | Source states/Native categories; игровая проверка ожидается |
| 330 | .lake_slot0 | dora3_fisher1_update_and_limit_depth_8021E240 | enemy | Source states/Native categories; игровая проверка ожидается |
| 331 | .lake_slot0 | dora3_fisher2_update_8021E994 | enemy | Source states/Native categories; игровая проверка ожидается |
| 332 | .lake_slot0 | dora3_yadokaringer1_update_8021F5D0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 333 | .lake_slot0 | dora3_yadokaringer_bubble1_update_8021FB68 | enemy | Source states/Native categories; игровая проверка ожидается |
| 334 | .lake_slot0 | dora3_yadokaringer2_update_802209C4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 335 | .lake_slot0 | dora3_yadokaringer_bubble2_update_80220F54 | enemy | Source states/Native categories; игровая проверка ожидается |
| 336 | .lake_slot0 | dora3_rikurage1_update_80221B54 | enemy | Source states/Native categories; игровая проверка ожидается |
| 337 | .lake_slot0 | dora3_rikurage2_update_80222AA0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 338 | .lake_slot0 | dora3_rikurage_light_update_80222C74 | enemy | Source states/Native categories; игровая проверка ожидается |
| 339 | .lake_slot0 | dora3_tatsunoorgo_update_802234D0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 340 | .lake_slot0 | dora3_tatsunoorgo_bubble_update_802239C0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 341 | .demo_slot0 | dora3_brobo_a100_update_8021E29C | enemy | Source states/Native categories; игровая проверка ожидается |
| 342 | .demo_slot0 | dora3_brobo_a101_update_8021EC7C | enemy | Source states/Native categories; игровая проверка ожидается |
| 343 | .demo_slot0 | dora3_brobo_a101_projectile_update_8021F044 | enemy | Source states/Native categories; игровая проверка ожидается |
| 344 | .demo_slot0 | dora3_brobo_a101_projectile_update_8021F044 | enemy | Source states/Native categories; игровая проверка ожидается |
| 345 | .demo_slot0 | dora3_brobo_a101_effect_update_8021F0A8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 346 | .demo_slot0 | dora3_brobo_a103_update_80223748 | enemy | Source states/Native categories; игровая проверка ожидается |
| 347 | .demo_slot0 | dora3_brobo_a104_update_8022438C | enemy | Source states/Native categories; игровая проверка ожидается |
| 348 | .remaining_slot0_03 | dora3_brobodc08_fake_t1_update_and_publish_child_clip_8021E118 | enemy | Source states/Native categories; игровая проверка ожидается |
| 349 | .remaining_slot0_03 | dora3_brobodc08_real_t1_update_8021E7E0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 350 | .remaining_slot0_03 | dora3_brobodc08_t2_update_8021F578 | enemy | Source states/Native categories; игровая проверка ожидается |
| 351 | .remaining_slot0_03 | dora3_brobodc08_t3_update_80220220 | enemy | Source states/Native categories; игровая проверка ожидается |
| 352 | .remaining_slot0_03 | dora3_brobodc08_bomb_update_80220544 | enemy | Source states/Native categories; игровая проверка ожидается |
| 353 | .remaining_slot0_03 | dora3_brobodc08_hob_update_802207B8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 354 | .cave_slot2 | dora3_t00_shield_controller_update_8023DC5C | enemy | Source states/Native categories; игровая проверка ожидается |
| 355 | .cave_slot2 | dora3_t00_shield_effect_update_8023D7C4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 356 | .cave_slot2 | dora3_t00_shield_completed_update_8023D8D8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 357 | .demo_slot2 | dora3_t17_door_controller_update_8023E4C0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 358 | .demo_slot2 | dora3_t17_door_completed_update_8023E13C | enemy | Source states/Native categories; игровая проверка ожидается |
| 359 | .level1_slot0 | dora3_minicar_update_8021E600 | enemy | Source states/Native categories; игровая проверка ожидается |
| 360 | .level1_slot0 | dora3_minicar2_update_8021F428 | enemy | Source states/Native categories; игровая проверка ожидается |
| 361 | .level1_slot0 | dora3_minicar_smoke_update_8021F618 | enemy | Source states/Native categories; игровая проверка ожидается |
| 362 | .level1_slot0 | dora3_patpot_update_80220538 | enemy | Source states/Native categories; игровая проверка ожидается |
| 363 | .level1_slot0 | dora3_patpot_projectile_update_802207C8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 364 | .remaining_slot1_04 | dora3_radicon_robo_update_8022DEA8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 365 | .remaining_slot1_04 | dora3_radicon_car_update_8022E460 | enemy | Source states/Native categories; игровая проверка ожидается |
| 366 | .lake_slot1 | dora3_sentoutei1_update_and_floor_offset_8022EC80 | enemy | Source states/Native categories; игровая проверка ожидается |
| 367 | .lake_slot1 | dora3_sentoutei2_update_noop_8022EEC0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 368 | .lake_slot1 | dora3_sentoutei3_update_8022FDB8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 369 | .lake_slot1 | dora3_sentoutei_projectile_update_80230080 | enemy | Source states/Native categories; игровая проверка ожидается |
| 370 | .remaining_slot1_02 | dora3_kaendan_generator_update_8022FDD8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 371 | .remaining_slot1_02 | dora3_kaendan_projectile_update_802304C0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 372 | .remaining_slot1_02 | dora3_kaenhousha_generator_update_80230840 | enemy | Source states/Native categories; игровая проверка ожидается |
| 373 | .remaining_slot1_02 | dora3_kaenhousha_segment_update_80230FA0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 374 | .demo_slot1 | dora3_butterfly_fly_and_change_height_80230558 | enemy | Source states/Native categories; игровая проверка ожидается |
| 375 | .demo_slot1 | dora3_butterfly_fly_and_change_height_80230558 | enemy | Source states/Native categories; игровая проверка ожидается |
| 376 | .demo_slot1 | dora3_butterfly_fly_and_change_height_80230558 | enemy | Source states/Native categories; игровая проверка ожидается |
| 377 | .lake_slot2 | dora3_sakana_update_8023F628 | enemy | Source states/Native categories; игровая проверка ожидается |
| 378 | .lake_slot2 | dora3_sakana_update_8023F628 | enemy | Source states/Native categories; игровая проверка ожидается |
| 379 | .remaining_slot1_04 | dora3_thunder_generator_update_8022FDE0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 380 | .remaining_slot1_04 | dora3_thunder_warning_update_8022FAB8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 381 | .remaining_slot1_04 | dora3_thunder_spark_update_8022FBF0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 382 | .lake_slot1 | dora3_houdai_update_80230568 | enemy | Source states/Native categories; игровая проверка ожидается |
| 383 | .lake_slot1 | dora3_houdai_base_update_802305D4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 384 | .lake_slot1 | dora3_houdai_projectile_update_80230878 | enemy | Source states/Native categories; игровая проверка ожидается |
| 385 | .lake_slot1 | dora3_majin_update_80230B80 | enemy | Source states/Native categories; игровая проверка ожидается |
| 386 | .lake_slot1 | dora3_majin_spirit_update_80230EC8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 387 | .level1_slot0 | dora3_powan1_update_80220E10 | enemy | Source states/Native categories; игровая проверка ожидается |
| 388 | .level1_slot0 | dora3_powan2_update_802218A8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 389 | .level1_slot0 | dora3_powan3_update_80222608 | enemy | Source states/Native categories; игровая проверка ожидается |
| 390 | .level1_slot0 | dora3_powan4_update_80223744 | enemy | Source states/Native categories; игровая проверка ожидается |
| 391 | .level1_slot0 | dora3_powan_transform_effect_update_80223880 | enemy | Source states/Native categories; игровая проверка ожидается |
| 392 | .lake_slot1 | dora3_iwa_update_80231014 | enemy | Source states/Native categories; игровая проверка ожидается |
| 393 | .lake_slot2 | dora3_posedo_controller_update_8024046C | enemy | Source states/Native categories; игровая проверка ожидается |
| 394 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | enemy | Source states/Native categories; игровая проверка ожидается |
| 395 | .remaining_slot1_06 | dora3_kirafu_update_8022ECBC | enemy | Source states/Native categories; игровая проверка ожидается |
| 396 | .remaining_slot1_06 | dora3_kirafu_smoke_animate_until_unlinked_8022ED4C | enemy | Source states/Native categories; игровая проверка ожидается |
| 397 | .remaining_slot1_06 | dora3_kirafu_particle_update_8022F054 | enemy | Source states/Native categories; игровая проверка ожидается |
| 398 | .lake_slot1 | dora3_jyaicong_update_and_project_orbit_8023179C | enemy | Source states/Native categories; игровая проверка ожидается |
| 399 | .remaining_slot0_11 | dora3_boss1_body_update_health_ui_and_dispatch_8021EB4C | enemy | Source states/Native categories; игровая проверка ожидается |
| 400 | .remaining_slot0_11 | dora3_boss1_hole_update_8021EDF0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 401 | .remaining_slot0_11 | dora3_boss1_shield_follow_parent_and_update_8021F140 | enemy | Source states/Native categories; игровая проверка ожидается |
| 402 | .remaining_slot0_11 | dora3_boss1_lightning_update_8021F464 | enemy | Source states/Native categories; игровая проверка ожидается |
| 403 | .remaining_slot0_11 | dora3_boss1_water_pole_update_publish_old_alpha_8021FD84 | enemy | Source states/Native categories; игровая проверка ожидается |
| 404 | .remaining_slot0_11 | dora3_boss1_splash_update_8021FE94 | enemy | Source states/Native categories; игровая проверка ожидается |
| 405 | .remaining_slot0_11 | dora3_boss1_vortex_spin_and_update_802203BC | enemy | Source states/Native categories; игровая проверка ожидается |
| 406 | .remaining_slot0_11 | dora3_boss1_vortex_trail_update_80220654 | enemy | Source states/Native categories; игровая проверка ожидается |
| 407 | .remaining_slot0_11 | dora3_boss1_tsunami_update_802208D8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 408 | .remaining_slot0_11 | dora3_boss1_crash_update_80220A14 | enemy | Source states/Native categories; игровая проверка ожидается |
| 409 | .remaining_slot0_11 | dora3_boss1_fire_pot_update_80220B28 | enemy | Source states/Native categories; игровая проверка ожидается |
| 410 | .remaining_slot0_12 | dora3_boss2_head_health_clip_shell_and_state_dispatch_80228994 | enemy | Source states/Native categories; игровая проверка ожидается |
| 411 | .remaining_slot0_12 | dora3_boss2_body_update_8021E814 | enemy | Source states/Native categories; игровая проверка ожидается |
| 412 | .remaining_slot0_12 | dora3_boss2_leg_update_8021FE0C | enemy | Source states/Native categories; игровая проверка ожидается |
| 413 | .remaining_slot0_12 | dora3_boss2_spark_clip_and_retire_8021FEC8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 414 | .remaining_slot0_12 | dora3_boss2_rocket_update_802204FC | enemy | Source states/Native categories; игровая проверка ожидается |
| 415 | .remaining_slot0_12 | dora3_boss2_arm_flash_update_802206B4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 416 | .remaining_slot0_12 | dora3_boss2_smoke_fade_grow_and_move_80220718 | enemy | Source states/Native categories; игровая проверка ожидается |
| 417 | .remaining_slot0_12 | dora3_boss2_smoke_fade_grow_and_move_80220718 | enemy | Source states/Native categories; игровая проверка ожидается |
| 418 | .remaining_slot0_12 | dora3_boss2_bomb_update_80220988 | enemy | Source states/Native categories; игровая проверка ожидается |
| 419 | .remaining_slot0_12 | dora3_boss2_shot_update_80220B28 | enemy | Source states/Native categories; игровая проверка ожидается |
| 420 | .remaining_slot0_12 | dora3_boss2_laser_parent_matrix_and_update_80220D48 | enemy | Source states/Native categories; игровая проверка ожидается |
| 421 | .remaining_slot0_12 | dora3_boss2_laser_effect_parent_matrix_and_update_80220FA8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 422 | .remaining_slot0_12 | dora3_boss2_missile_update_802212E0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 423 | .remaining_slot0_12 | dora3_boss2_fannel_update_target_angles_and_dispatch_80221FE4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 424 | .remaining_slot0_12 | dora3_boss2_fannel_laser_update_8022223C | enemy | Source states/Native categories; игровая проверка ожидается |
| 425 | .remaining_slot0_12 | dora3_boss2_fire_follow_selected_part_and_update_8022245C | enemy | Source states/Native categories; игровая проверка ожидается |
| 426 | .remaining_slot0_12 | dora3_boss2_back_flash_size_clip_and_retire_802225BC | enemy | Source states/Native categories; игровая проверка ожидается |
| 427 | .remaining_slot0_12 | dora3_boss2_crash_clip_threshold_fade_reward_and_retire_80222674 | enemy | Source states/Native categories; игровая проверка ожидается |
| 428 | .remaining_slot0_13 | dora3_boss3_body_health_clip_and_state_dispatch_80220EAC | enemy | Source states/Native categories; игровая проверка ожидается |
| 429 | .remaining_slot0_13 | dora3_boss3_shot_update_8021DAD4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 430 | .remaining_slot0_13 | dora3_boss3_sound_wave_update_8021DDC8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 431 | .remaining_slot0_13 | dora3_boss3_pipe_update_8021E078 | enemy | Source states/Native categories; игровая проверка ожидается |
| 432 | .remaining_slot0_13 | dora3_boss3_apple_side_clip_and_retire_8021E120 | enemy | Source states/Native categories; игровая проверка ожидается |
| 433 | .remaining_slot0_13 | dora3_boss3_plate_update_8021E4A4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 434 | .remaining_slot0_13 | dora3_boss3_crash_shrink_fade_reward_and_retire_8021E534 | enemy | Source states/Native categories; игровая проверка ожидается |
| 435 | .remaining_slot0_14 | dora3_boss4_body_health_children_and_state_dispatch_8022243C | enemy | Source states/Native categories; игровая проверка ожидается |
| 436 | .remaining_slot0_14 | dora3_boss4_suzu_update_8021DB50 | enemy | Source states/Native categories; игровая проверка ожидается |
| 437 | .remaining_slot0_14 | dora3_boss4_hitode_update_8021E288 | enemy | Source states/Native categories; игровая проверка ожидается |
| 438 | .remaining_slot0_14 | dora3_boss4_doka_update_8021E808 | enemy | Source states/Native categories; игровая проверка ожидается |
| 439 | .remaining_slot0_14 | dora3_boss4_room_controller_update_8021E9E0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 440 | .remaining_slot0_14 | dora3_boss4_time_hole_update_8021EDF0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 441 | .remaining_slot0_14 | dora3_boss4_time_machine_update_8021EF6C | enemy | Source states/Native categories; игровая проверка ожидается |
| 442 | .remaining_slot0_14 | dora3_boss4_fire_ball_wait_grow_rise_home_and_accelerate_8021EFD0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 443 | .remaining_slot0_14 | dora3_boss4_shot_update_8021F4DC | enemy | Source states/Native categories; игровая проверка ожидается |
| 444 | .remaining_slot0_14 | dora3_boss4_smoke_fade_grow_and_drift_8021F540 | enemy | Source states/Native categories; игровая проверка ожидается |
| 445 | .remaining_slot0_14 | dora3_boss4_crash_clip_shrink_and_retire_8021F634 | enemy | Source states/Native categories; игровая проверка ожидается |
| 446 | .remaining_slot0_15 | dora3_boss5_body_health_clip_dispatch_and_probe_80221674 | enemy | Source states/Native categories; игровая проверка ожидается |
| 447 | .remaining_slot0_15 | dora3_boss5_slime_follow_parent_or_retire_8021D6E0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 448 | .remaining_slot0_15 | dora3_boss5_x_beam_update_8021DA94 | enemy | Source states/Native categories; игровая проверка ожидается |
| 449 | .remaining_slot0_15 | dora3_boss5_laser_parent_matrix_pitch_and_dispatch_8021DD18 | enemy | Source states/Native categories; игровая проверка ожидается |
| 450 | .remaining_slot0_15 | dora3_boss5_laser_flare_update_8021E100 | enemy | Source states/Native categories; игровая проверка ожидается |
| 451 | .remaining_slot0_15 | dora3_boss5_shield_update_8021E59C | enemy | Source states/Native categories; игровая проверка ожидается |
| 452 | .remaining_slot0_15 | dora3_boss5_shadow_elliptic_phase_pitch_and_retire_8021E634 | enemy | Source states/Native categories; игровая проверка ожидается |
| 453 | .remaining_slot0_15 | dora3_boss5_crash_grow_shrink_fade_spin_and_reward_8021E9AC | enemy | Source states/Native categories; игровая проверка ожидается |
| 454 | .remaining_slot0_16 | dora3_boss6_head_health_clip_shell_and_state_dispatch_80228098 | enemy | Source states/Native categories; игровая проверка ожидается |
| 455 | .remaining_slot0_16 | dora3_boss6_body_update_8021E738 | enemy | Source states/Native categories; игровая проверка ожидается |
| 456 | .remaining_slot0_16 | dora3_boss6_leg_update_8021F914 | enemy | Source states/Native categories; игровая проверка ожидается |
| 457 | .remaining_slot0_16 | dora3_boss6_spark_play_and_retire_8021F9D0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 458 | .remaining_slot0_16 | dora3_boss6_rocket_update_80220004 | enemy | Source states/Native categories; игровая проверка ожидается |
| 459 | .remaining_slot0_16 | dora3_boss6_arm_flash_update_802201BC | enemy | Source states/Native categories; игровая проверка ожидается |
| 460 | .remaining_slot0_16 | dora3_boss6_white_black_smoke_fade_grow_and_drift_80220220 | enemy | Source states/Native categories; игровая проверка ожидается |
| 461 | .remaining_slot0_16 | dora3_boss6_white_black_smoke_fade_grow_and_drift_80220220 | enemy | Source states/Native categories; игровая проверка ожидается |
| 462 | .remaining_slot0_16 | dora3_boss6_bomb_update_80220490 | enemy | Source states/Native categories; игровая проверка ожидается |
| 463 | .remaining_slot0_16 | dora3_boss6_shot_update_80220630 | enemy | Source states/Native categories; игровая проверка ожидается |
| 464 | .remaining_slot0_16 | dora3_boss6_laser_parent_matrix_and_update_80220850 | enemy | Source states/Native categories; игровая проверка ожидается |
| 465 | .remaining_slot0_16 | dora3_boss6_laser_effect_parent_matrix_and_update_80220AB0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 466 | .remaining_slot0_16 | dora3_boss6_missile_update_80220DE8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 467 | .remaining_slot0_16 | dora3_boss6_fannel_look_at_player_and_state_dispatch_80221A1C | enemy | Source states/Native categories; игровая проверка ожидается |
| 468 | .remaining_slot0_16 | dora3_boss6_fannel_laser_update_80221C54 | enemy | Source states/Native categories; игровая проверка ожидается |
| 469 | .remaining_slot0_16 | dora3_boss6_fire_selected_parent_pose_and_dispatch_80221E74 | enemy | Source states/Native categories; игровая проверка ожидается |
| 470 | .remaining_slot0_16 | dora3_boss6_back_flash_scale_clip_and_retire_80221FD4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 471 | .remaining_slot0_16 | dora3_boss6_crash_frame_fade_progress_and_retire_8022208C | enemy | Source states/Native categories; игровая проверка ожидается |
| 472 | .remaining_slot0_17 | dora3_boss7_bug_health_and_full_state_dispatch_8021F048 | enemy | Source states/Native categories; игровая проверка ожидается |
| 473 | .remaining_slot0_17 | dora3_boss7_mask_follow_bug_pose_and_retire_8021D6E0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 474 | .remaining_slot0_17 | dora3_boss7_slime_state_dispatch_8021D7B4 | enemy | Source states/Native categories; игровая проверка ожидается |
| 475 | .remaining_slot0_17 | dora3_boss7_shield_active_death_dispatch_8021DA74 | enemy | Source states/Native categories; игровая проверка ожидается |
| 476 | .remaining_slot0_17 | dora3_boss7_crash_frame_fade_reward_and_retire_8021DAD8 | enemy | Source states/Native categories; игровая проверка ожидается |
| 477 | .remaining_slot0_17 | dora3_boss7_wall_encounter_show_hide_8021DCE0 | enemy | Source states/Native categories; игровая проверка ожидается |
| 478 | .level1_slot2 | dora3_brobo_a100_break_update_noop_8023D944 | enemy | Source states/Native categories; игровая проверка ожидается |
| 479 | .remaining_slot2_08 | dora3_scene_object_m023__paint_block_update_8023EF1C | scene | Source states/Native categories; игровая проверка ожидается |
| 480 | .remaining_slot2_08 | dora3_scene_object_m024__bridge_piece_update_8023F268 | scene | Source states/Native categories; игровая проверка ожидается |
| 481 | .scene1 | dora3_scene_object_exit_mark_update_801EB440 | scene | Source states/Native categories; игровая проверка ожидается |
| 482 | .scene1 | dora3_scene_object_exit_mark_update_801EB440 | scene | Source states/Native categories; игровая проверка ожидается |
| 483 | .scene1 | dora3_scene_object_warp_hole1_update_801EAC9C | scene | Source states/Native categories; игровая проверка ожидается |
| 484 | .scene1 | dora3_scene_object_warp_hole2_update_801EAD74 | scene | Source states/Native categories; игровая проверка ожидается |
| 485 | .scene1 | dora3_scene_object_treasure_box1_update_801EB7BC | scene | Source states/Native categories; игровая проверка ожидается |
| 486 | .scene1 | dora3_scene_object_treasure_box1_update_801EB7BC | scene | Source states/Native categories; игровая проверка ожидается |
| 487 | .scene1 | dora3_scene_object_treasure_box1_update_801EB7BC | scene | Source states/Native categories; игровая проверка ожидается |
| 488 | .remaining_slot2_08 | dora3_scene_object_sound_switch_update_8023DBD0 | scene | Source states/Native categories; игровая проверка ожидается |
| 489 | .cave_slot2 | dora3_scene_object_domero_switch_update_8023E798 | scene | Source states/Native categories; игровая проверка ожидается |
| 490 | .demo_slot2 | dora3_scene_object_mariru_plate_update_8023E6C0 | scene | Source states/Native categories; игровая проверка ожидается |
| 491 | .demo_slot2 | dora3_scene_object_mariru_switch_update_8023E94C | scene | Source states/Native categories; игровая проверка ожидается |
| 492 | .demo_slot2 | dora3_scene_object_dancing_flower_update_8023EA4C | scene | Source states/Native categories; игровая проверка ожидается |
| 493 | .mountain_slot1 | dora3_scene_object_m002__door00_update_8022EDE8 | scene | Source states/Native categories; игровая проверка ожидается |
| 494 | .mountain_slot1 | dora3_scene_object_m002__door01_update_8022EF9C | scene | Source states/Native categories; игровая проверка ожидается |
| 495 | .lake_slot2 | dora3_scene_object_m003__door00_update_8023F93C | scene | Source states/Native categories; игровая проверка ожидается |
| 496 | .demo_slot2 | dora3_scene_object_m004__door00_update_8023E0D8 | scene | Source states/Native categories; игровая проверка ожидается |
| 497 | .cave_slot2 | dora3_scene_object_m006__door01_update_8023DF60 | scene | Source states/Native categories; игровая проверка ожидается |
| 498 | .cave_slot2 | dora3_scene_object_m006__door03_update_8023E330 | scene | Source states/Native categories; игровая проверка ожидается |
| 499 | .cave_slot2 | dora3_scene_object_m007__door01_update_8023E9A8 | scene | Source states/Native categories; игровая проверка ожидается |
| 500 | .cave_slot2 | dora3_scene_object_m007__shield_update_8023EB18 | scene | Source states/Native categories; игровая проверка ожидается |
| 501 | .remaining_slot2_04 | dora3_scene_object_m009__door00_update_8023DA00 | scene | Source states/Native categories; игровая проверка ожидается |
| 502 | .remaining_slot2_04 | dora3_scene_object_m009__door01_update_8023DD4C | scene | Source states/Native categories; игровая проверка ожидается |
| 503 | .remaining_slot2_04 | dora3_scene_object_m009__door02_update_8023E098 | scene | Source states/Native categories; игровая проверка ожидается |
| 504 | .remaining_slot2_04 | dora3_scene_object_m009__door03_update_8023E3F4 | scene | Source states/Native categories; игровая проверка ожидается |
| 505 | .penguin_slot2 | dora3_scene_object_m010__fusya_update_8023D840 | scene | Source states/Native categories; игровая проверка ожидается |
| 506 | .penguin_slot2 | dora3_scene_object_m010__light_update_8023D8E4 | scene | Source states/Native categories; игровая проверка ожидается |
| 507 | .penguin_slot2 | dora3_scene_object_m010__monorail_update_8023DCD8 | scene | Source states/Native categories; игровая проверка ожидается |
| 508 | .remaining_slot2_04 | dora3_scene_object_m015__door00_update_8023E7A4 | scene | Source states/Native categories; игровая проверка ожидается |
| 509 | .remaining_slot2_04 | dora3_scene_object_m015__door01_update_8023EB54 | scene | Source states/Native categories; игровая проверка ожидается |
| 510 | .remaining_slot2_04 | dora3_scene_object_m015__door02_update_8023EEA0 | scene | Source states/Native categories; игровая проверка ожидается |
| 511 | .remaining_slot1_02 | dora3_scene_object_m016__dai_update_8022F7F0 | scene | Source states/Native categories; игровая проверка ожидается |
| 512 | .lake_slot1 | dora3_scene_object_m020__iga_head_update_8022DF54 | scene | Source states/Native categories; игровая проверка ожидается |
| 513 | .demo_slot2 | dora3_scene_object_m025__crystal_update_8023DE34 | scene | Source states/Native categories; игровая проверка ожидается |
| 514 | .remaining_slot2_08 | dora3_scene_object_sound_door_update_8023E4C0 | scene | Source states/Native categories; игровая проверка ожидается |
| 515 | .remaining_slot2_08 | dora3_scene_object_sound_light_blue_update_8023E334 | scene | Source states/Native categories; игровая проверка ожидается |
| 516 | .remaining_slot2_08 | dora3_scene_object_sound_light_blue_update_8023E334 | scene | Source states/Native categories; игровая проверка ожидается |
| 517 | .remaining_slot2_08 | dora3_scene_object_sound_light_blue_update_8023E334 | scene | Source states/Native categories; игровая проверка ожидается |
| 518 | .scene1 | dora3_scene_object_container_update_801EA470 | scene | Source states/Native categories; игровая проверка ожидается |
| 519 | .demo_slot2 | dora3_scene_object_spring1_update_8023D830 | scene | Source states/Native categories; игровая проверка ожидается |
| 520 | .mountain_slot2 | dora3_scene_object_kumo_update_8023DEA4 | scene | Source states/Native categories; игровая проверка ожидается |
| 521 | .remaining_slot2_07 | dora3_scene_object_taiyo_update_8023D768_rom_159780 | scene | Source states/Native categories; игровая проверка ожидается |
| 521 | .level1_slot2 | dora3_scene_object_taiyo_update_8023D768_rom_15E440 | scene | Source states/Native categories; игровая проверка ожидается |
| 522 | .mountain_slot2 | dora3_scene_object_jack_kuki_update_8023D6E0 | scene | Source states/Native categories; игровая проверка ожидается |
| 523 | .mountain_slot2 | dora3_scene_object_jack_leaf_r_update_8023D7D0 | scene | Source states/Native categories; игровая проверка ожидается |
| 524 | .mountain_slot2 | dora3_scene_object_jack_tuchi_update_8023DB44 | scene | Source states/Native categories; игровая проверка ожидается |
| 525 | .remaining_slot1_04 | dora3_scene_object_kaiten_dai_update_8022EB44 | scene | Source states/Native categories; игровая проверка ожидается |
| 526 | .remaining_slot1_04 | dora3_scene_object_kaiten_pole_update_8022EC00 | scene | Source states/Native categories; игровая проверка ожидается |
| 527 | .remaining_slot1_04 | dora3_scene_object_kaiten_button_update_8022EC18 | scene | Source states/Native categories; игровая проверка ожидается |
| 528 | .remaining_slot1_04 | dora3_scene_object_kaiten_glass_update_8022ED9C | scene | Source states/Native categories; игровая проверка ожидается |
| 529 | .remaining_slot1_04 | dora3_scene_object_control_stone_update_8022F15C | scene | Source states/Native categories; игровая проверка ожидается |
| 530 | .remaining_slot1_04 | dora3_scene_object_metarudo_door1_update_8022F680 | scene | Source states/Native categories; игровая проверка ожидается |
| 531 | .remaining_slot1_04 | dora3_scene_object_metarudo_door1_update_8022F680 | scene | Source states/Native categories; игровая проверка ожидается |
| 532 | .remaining_slot1_04 | dora3_scene_object_metarudo_panel1_update_8022F944 | scene | Source states/Native categories; игровая проверка ожидается |
| 533 | .remaining_slot1_04 | dora3_scene_object_metarudo_panel1_update_8022F944 | scene | Source states/Native categories; игровая проверка ожидается |
| 534 | .remaining_slot1_04 | dora3_scene_object_tower_flash_update_8022F9EC | scene | Source states/Native categories; игровая проверка ожидается |
| 535 | .scene1 | dora3_scene_object_switch_base_update_801EAE54 | scene | Source states/Native categories; игровая проверка ожидается |
| 536 | .scene1 | dora3_scene_object_switch_light_update_801EAE6C | scene | Source states/Native categories; игровая проверка ожидается |
| 537 | .remaining_slot2_08 | dora3_scene_object_stone_arm_update_8023E700 | scene | Source states/Native categories; игровая проверка ожидается |
| 538 | .remaining_slot1_02 | dora3_scene_object_trap_dai_update_8022FA84 | scene | Source states/Native categories; игровая проверка ожидается |
| 539 | .remaining_slot1_02 | dora3_scene_object_trap_fire_update_8022FCE8 | scene | Source states/Native categories; игровая проверка ожидается |
| 540 | .remaining_slot2_08 | dora3_scene_object_speaker_update_8023E96C | scene | Source states/Native categories; игровая проверка ожидается |
| 541 | .remaining_slot2_07 | dora3_scene_object_water_drop_update_8023D844_rom_159780 | scene | Source states/Native categories; игровая проверка ожидается |
| 541 | .level1_slot2 | dora3_scene_object_water_drop_update_8023D844_rom_15E440 | scene | Source states/Native categories; игровая проверка ожидается |
| 542 | .remaining_slot2_08 | dora3_scene_object_m022__dai_update_8023EBD8 | scene | Source states/Native categories; игровая проверка ожидается |
| 543 | .level1_slot2 | dora3_scene_object_saku_update_8023DB14 | scene | Source states/Native categories; игровая проверка ожидается |
| 544 | .scene1 | dora3_scene_object_yajirusi_update_801EAF68 | scene | Source states/Native categories; игровая проверка ожидается |
| 545 | .remaining_slot2_07 | dora3_scene_object_taiyo_update_8023D768_rom_159780 | scene | Source states/Native categories; игровая проверка ожидается |
| 545 | .level1_slot2 | dora3_scene_object_taiyo_update_8023D768_rom_15E440 | scene | Source states/Native categories; игровая проверка ожидается |
| 546 | .level1_slot2 | dora3_scene_object_book2_update_8023D854 | scene | Source states/Native categories; игровая проверка ожидается |
| 547 | .scene1 | dora3_scene_object_m051__sibuki_update_801EB0EC | scene | Source states/Native categories; игровая проверка ожидается |
| 548 | .remaining_slot2_07 | dora3_scene_object_ice_plate_big_update_8023DD98 | scene | Source states/Native categories; игровая проверка ожидается |
| 549 | .remaining_slot2_07 | dora3_scene_object_ice_plate_small_update_8023DEA0 | scene | Source states/Native categories; игровая проверка ожидается |
| 550 | .remaining_slot2_09 | dora3_scene_object_m101__plate_update_8023D85C | scene | Source states/Native categories; игровая проверка ожидается |
| 551 | .remaining_slot2_09 | dora3_scene_object_m102__plate_update_8023D938 | scene | Source states/Native categories; игровая проверка ожидается |
| 552 | .remaining_slot2_09 | dora3_scene_object_miire_switch_update_8023DBBC | scene | Source states/Native categories; игровая проверка ожидается |
| 553 | .remaining_slot2_09 | dora3_scene_object_m103__plate_update_8023DCCC | scene | Source states/Native categories; игровая проверка ожидается |
| 554 | .remaining_slot2_09 | dora3_scene_object_m103__plate_update_8023DCCC | scene | Source states/Native categories; игровая проверка ожидается |
| 555 | .remaining_slot2_09 | dora3_scene_object_miire_control_update_8023DF70 | scene | Source states/Native categories; игровая проверка ожидается |
| 556 | .remaining_slot2_07 | dora3_scene_object_snow_update_8023DFF4 | scene | Source states/Native categories; игровая проверка ожидается |
| 557 | .remaining_slot2_07 | dora3_scene_object_kogooru_switch_update_8023E4A0 | scene | Source states/Native categories; игровая проверка ожидается |
| 558 | .remaining_slot2_07 | dora3_scene_object_kogooru_plate_update_8023E7A4 | scene | Source states/Native categories; игровая проверка ожидается |
| 559 | .cave_slot2 | dora3_scene_object_ratena_plate_update_8023EDA4 | scene | Source states/Native categories; игровая проверка ожидается |
| 560 | .cave_slot2 | dora3_scene_object_ratena_switch1_update_8023F1C8 | scene | Source states/Native categories; игровая проверка ожидается |
| 561 | .cave_slot2 | dora3_scene_object_ratena_switch1_update_8023F1C8 | scene | Source states/Native categories; игровая проверка ожидается |
| 562 | .demo_slot2 | dora3_scene_object_wheel1_update_8023ECD8 | scene | Source states/Native categories; игровая проверка ожидается |
| 563 | .demo_slot2 | dora3_scene_object_wheel1_update_8023ECD8 | scene | Source states/Native categories; игровая проверка ожидается |
| 564 | .demo_slot2 | dora3_scene_object_gondola_update_8023ED3C | scene | Source states/Native categories; игровая проверка ожидается |
| 565 | .penguin_slot2 | dora3_scene_object_m087__haguruma_update_8023DDF4 | scene | Source states/Native categories; игровая проверка ожидается |
| 566 | .penguin_slot2 | dora3_scene_object_m088__plate_update_8023DEC4 | scene | Source states/Native categories; игровая проверка ожидается |
| 567 | .remaining_slot2_08 | dora3_scene_object_sound_tama_update_8023F5E4 | scene | Source states/Native categories; игровая проверка ожидается |
| 568 | .demo_slot2 | dora3_scene_object_m008__control_update_8023F138 | scene | Source states/Native categories; игровая проверка ожидается |
| 569 | .remaining_slot2_09 | dora3_scene_object_m107__plate1_update_8023E178 | scene | Source states/Native categories; игровая проверка ожидается |
| 570 | .remaining_slot2_09 | dora3_scene_object_m107__plate1_update_8023E178 | scene | Source states/Native categories; игровая проверка ожидается |
| 571 | .remaining_slot2_09 | dora3_scene_object_m107__plate3_update_8023E254 | scene | Source states/Native categories; игровая проверка ожидается |
| 572 | .remaining_slot2_09 | dora3_scene_object_baguru_switch1_update_8023E514 | scene | Source states/Native categories; игровая проверка ожидается |
| 573 | .remaining_slot2_09 | dora3_scene_object_baguru_switch1_update_8023E514 | scene | Source states/Native categories; игровая проверка ожидается |
| 574 | .demo_slot2 | dora3_scene_object_gondola_bridge_update_8023F214 | scene | Source states/Native categories; игровая проверка ожидается |
| 575 | .demo_slot2 | dora3_scene_object_dezu_switch_update_8023F3D0 | scene | Source states/Native categories; игровая проверка ожидается |
| 576 | .demo_slot2 | dora3_scene_object_dezu_plate_update_8023F59C | scene | Source states/Native categories; игровая проверка ожидается |
| 577 | .mountain_slot2 | dora3_scene_object_metarudo_shild_ctrl_update_8023E1E8 | scene | Source states/Native categories; игровая проверка ожидается |
| 578 | .lake_slot2 | dora3_scene_object_m003__door01_update_8023FB30 | scene | Source states/Native categories; игровая проверка ожидается |
| 579 | .remaining_slot1_01 | dora3_scene_object_m011__door00_update_8022F5BC | scene | Source states/Native categories; игровая проверка ожидается |
| 580 | .remaining_slot1_02 | dora3_scene_object_m014__door00_update_80231178 | scene | Source states/Native categories; игровая проверка ожидается |
| 581 | .remaining_slot1_02 | dora3_scene_object_m014__door01_update_80231320 | scene | Source states/Native categories; игровая проверка ожидается |
| 582 | .cave_slot2 | dora3_scene_object_majin_light_update_8023F61C | scene | Source states/Native categories; игровая проверка ожидается |
| 583 | .cave_slot2 | dora3_scene_object_majin_effect_update_8023F6AC | scene | Source states/Native categories; игровая проверка ожидается |
| 584 | .lake_slot1 | dora3_scene_object_m106__door_update_802313E8 | scene | Source states/Native categories; игровая проверка ожидается |
| 585 | .mountain_slot2 | dora3_scene_object_dorami_time_machine_update_8023E2D4 | scene | Source states/Native categories; игровая проверка ожидается |
| 586 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 587 | .level1_slot2 | dora3_scene_object_nobiheya_slime_update_8023DC78 | scene | Source states/Native categories; игровая проверка ожидается |
| 588 | .remaining_slot2_08 | dora3_scene_object_goddess_update_8023FC40 | scene | Source states/Native categories; игровая проверка ожидается |
| 589 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |
| 590 | .level1_slot2 | dora3_scene_object_nobiheya_drawer_update_8023E0B8 | scene | Source states/Native categories; игровая проверка ожидается |
| 591 | .scene1 | dora3_posedo_shield_update_noop_801E5D84 | scene | Source states/Native categories; игровая проверка ожидается |

## Итог сборки

Компиляция и упаковка148 успешны. CrashLog/Diagnostics/GraphicsProfile OFF; PDB/CodeView отсутствуют;617 существовавших файлов сохранены. Исправлена одна ошибка компиляции в RAM alias новой функции handoff_source_flags и продолжена инкрементальная сборка. Проверка пяти изменённых исходников и готового EXE по SHA256 прошла. Игровая проверка ожидается, commit/push не выполнены.

## Подтверждение владельца — 2026-10-13

Пользователь проверил test148 и сообщил, что софтлока больше нет. Разрешены
коммит и push накопленного блока147–148 в main. Это игровое подтверждение
устранения ранее сообщённого софтлока Boss5. Прохождение всех перечисленных
врагов/NPC и каждого состояния этим сообщением не подтверждается.
Предшествующие записи об ожидании проверки оставлены как история этапа.
Агент не запускал игру или тесты; для этой документальной записи новой сборки нет.
