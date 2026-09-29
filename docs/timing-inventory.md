# Общая карта перевода времени

Автоматический статический инвентарь по закреплённому ROM. Числа показывают объём найденного кода, а не процент готовности native FPS.

Охвачено 4778 функций в 50 секциях; найдено 50 вызовов установки callbacks и 61 косвенных вызовов.

## Общие подсистемы

| Подсистема | Прямых вызовов |
|---|---:|
| fade_command | 167 |
| scene_select | 43 |
| animation_pose_and_time | 36 |
| input_select | 25 |
| input_repeat | 22 |
| callback_pair | 47 |
| cpu_framebuffer_copy | 1 |
| callback_slot | 3 |
| animation_pose | 1 |
| animation_time | 1 |
| animation_seek | 1 |
| animation_clamp | 1 |
| animation_pause | 1 |
| dispatch | 2 |
| cross_frame_copy | 1 |
| fade_tick | 1 |
| rumble | 1 |
| controller_pak | 1 |
| sound_events | 1 |
| resource_load | 1 |
| scene_unload | 1 |

## Секции и границы callbacks

| Секция | Функций | Установок callbacks | Косвенных вызовов |
|---|---:|---:|---:|
| .startup | 697 | 4 | 48 |
| .scene_common | 69 | 0 | 0 |
| .scene34 | 152 | 13 | 0 |
| .scene1 | 1303 | 3 | 12 |
| .demo_slot0 | 77 | 0 | 0 |
| .demo_slot1 | 40 | 0 | 0 |
| .demo_slot2 | 64 | 0 | 0 |
| .level1_slot0 | 72 | 0 | 0 |
| .level1_slot1 | 75 | 0 | 0 |
| .level1_slot2 | 29 | 0 | 0 |
| .cave_slot0 | 25 | 0 | 0 |
| .cave_slot1 | 31 | 0 | 0 |
| .cave_slot2 | 60 | 0 | 0 |
| .mountain_slot1 | 32 | 0 | 0 |
| .mountain_slot2 | 26 | 0 | 0 |
| .lake_slot0 | 64 | 0 | 0 |
| .lake_slot1 | 69 | 0 | 0 |
| .lake_slot2 | 52 | 0 | 0 |
| .penguin_slot1 | 74 | 0 | 0 |
| .penguin_slot2 | 30 | 0 | 0 |
| .minigame_common | 99 | 6 | 0 |
| .minigame_1 | 82 | 2 | 0 |
| .minigame_2 | 65 | 2 | 0 |
| .minigame_3 | 71 | 2 | 0 |
| .minigame_4 | 99 | 2 | 0 |
| .minigame_5 | 41 | 2 | 0 |
| .minigame_6 | 54 | 2 | 0 |
| .minigame_7 | 47 | 2 | 0 |
| .minigame_8 | 77 | 2 | 1 |
| .minigame_9 | 60 | 2 | 0 |
| .remaining_slot0_03 | 40 | 0 | 0 |
| .remaining_slot0_04 | 58 | 0 | 0 |
| .remaining_slot0_05 | 59 | 0 | 0 |
| .remaining_slot0_11 | 62 | 0 | 0 |
| .remaining_slot0_12 | 104 | 0 | 0 |
| .remaining_slot0_13 | 42 | 0 | 0 |
| .remaining_slot0_14 | 57 | 0 | 0 |
| .remaining_slot0_15 | 43 | 0 | 0 |
| .remaining_slot0_16 | 103 | 0 | 0 |
| .remaining_slot0_17 | 31 | 0 | 0 |
| .remaining_slot1_01 | 28 | 0 | 0 |
| .remaining_slot1_02 | 59 | 0 | 0 |
| .remaining_slot1_04 | 58 | 0 | 0 |
| .remaining_slot1_06 | 31 | 0 | 0 |
| .remaining_slot1_09 | 68 | 0 | 0 |
| .remaining_slot2_04 | 42 | 0 | 0 |
| .remaining_slot2_07 | 30 | 0 | 0 |
| .remaining_slot2_08 | 52 | 0 | 0 |
| .remaining_slot2_09 | 31 | 0 | 0 |
| .startup_scene | 44 | 6 | 0 |

## Как использовать

Сначала общий loop: независимый графический deadline, настоящие завершения, Original/Manual, пауза и смена сцен. Затем общие системы fade, model time/pose, transform/movement, camera, events и input. Только остаточные правила относятся к отдельным сценам и минииграм.

Каждая найденная пара update/draw имеет прямую closure в research/timing-inventory.json. Наличие stores/float operations — признак для разбора, не доказательство зависимости от FPS. Косвенные вызовы, таблицы callbacks, aliasing и семантика событий остаются открытыми; локальные константы не заменяют контроль потока.

Не следует увеличивать частоту всего dispatcher: он меняет integer waits и состояния process slots. Физике нужен общий шаг времени с сохранением частоты дискретных событий; анимации — порядок pose/time и однократность crossings; вводу — разделение held/edge/repeat; загрузке — drain до освобождения ресурсов.
