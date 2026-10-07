# Вылет при выходе через гаджет-дверь после052

2026-10-07. Пользователь подтвердил исправление нарастающей аудиозадержки
и катсцены Сидзуки, затем сообщил вылет при выходе из уровня через дверь.
Исходное состояние main4384d11, рабочее дерево чистое; последний EXE052.

## Доказательства

Дамп пользовательского запуска:
C:/Users/black/AppData/Local/CrashDumps/Dora64-3-test-052.exe.1000.dmp.
runtime.log: D:/Games/DoraRecomp3/testbuild/logs/1791383887487/runtime.log.
Последний переход: SCENE SELECT id=30 state=3. Последний новый ENTER801D4954
лишь пишет -1 в short802181DA; это не место падения.

Offline LLDB показывает C0000409/abort после CxxThrowException:
FadeTimeline::sample_at (RVA75AD18), sample(Nanoseconds) (75ABB3),
synchronize (69A3FD), игровой message loop (C54D4).
Дизассемблированный caller выбирает render_time для финального fade sample
при выходе из Native. Исключение: Fade clock moved backwards or invalid fraction.
previous_ns=0x2B999C617694, переданный now=0x2B999C360E78, обе дроби0.
Обратный ход=2844700нс (~2.845мс), pending_command=true.
Дамп прочитан без запуска игры; это не новая автоматическая проверка.

## Причина и изменение

Обычный Source callback может вызвать fade command между нативными кадрами,
а затем изменить scene/eligibility. Его command timestamp — текущее время
события. Следующий dora3_native_loop_message вызывает synchronize с новым
monotonic_time, но прежний код при выходе подменял его временем последней
позы render_time. Оно старше уже выполненной команды, что видно в дампе.
Это другой путь, чем suspended NPC callback, исправленный в052.

В src/native_game_loop.cpp финальный sample при смене mode/pause/scene
использует переданный synchronize timestamp события now. Обычный build_frame
по-прежнему семплирует fade вместе с позой на render_time; resource barrier
сохраняет точный Source command stamp. Обновление внутри native кадра по-прежнему
использует его now и общий мировой command clock из052.
Строгий guard FadeTimeline сохранён; timestamp не зажимается max/clamp,
Original scene selection и ROM bodies не менялись.

## Статус

Исправление внесено по дампу и статическому анализу. Тесты, игра, эмулятор,
renderer/GPU, генерация и новая сборка не запускались. В EXE052 этой правки нет.
Билд — после прямой команды пользователя или ответа «нет» на новые баги.
Отсутствие вылета после исправления ещё не подтверждено игровым запуском.
