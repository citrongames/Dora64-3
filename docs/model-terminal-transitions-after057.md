# Краткий возврат старой позы при смене анимации после057

2026-10-07. База main affd7a5, чистое дерево, последний Windows EXE057.
Пользователь заметил несколько кадров другой позы после появления врага из
ракушки в башне босса первого мира; поручил проверить общий механизм всех моделей.
Тесты, игровые прогоны, renderer/GPU не запускались.

## Установленный механизм

SDK8002A1B8 вычисляет pose800393C4, затем advance8003948C. Unclamped counter
после конца клипа возвращается к началу, а Source FSM принимает следующую
clip/state/pause/seek/retirement команду с исходной частотой. Native simulation
делит движение по настоящим clip boundaries, но callback AI не выполняет второй
Source command pass внутри того же периода. Между ними render брал wrapped
model+64 и показывал начало предыдущего клипа.

Прежняя TerminalPoseIntent из049 защищала38 точно разобранных Source-запросов
миниигр, только component0 и положительный Source rate. В Native ветви
ModelAnimation::begin_frame этот intent вообще не применялся. Поэтому даже
расширение старого списка без Native render fix было бы недостаточно.

Подтверждённые shell-related пути: Yadokaringer1 в .lake_slot0/ROM E5480,
query8021ED60 выбирает clip2 после окончания clip0;8021F000 возвращает state1
после clip4;8021F330 выбирает clip2 и снимает bubble links после clip3.
Tutaru326 в .mountain_slot1 query8022D8D4 также выбирает следующий clip после
appearance. Оба семейства включены. Точный descriptor видимого пользователю
врага не установлен одним описанием места; игровое воспроизведение не заявляется.

## Полный статический обзор общих API

Прочитаны все4778 compiled Original functions/50 sections закреплённого ROM.
Найдены362 object end-query sites в267 caller functions плюс1 вызов leaf getter
из общей SDK обёртки. Это call sites, не число моделей и не runtime coverage.
Общая инвентаризация также содержит752 выбора клипа,36 pose/advance callers,
48 seek,90 pause,222 clamp: эти команды уже инвалидируют observation/run и
настоящие Source counters/flags. Их существующие guards и ABI сохранены.

В recomp/model-terminal-poses.json записан каждый end-query с overlay ROM,
адресом/размером/хэшем owner body, JAL/delay/return и presentation policy:

-325 terminal sites в253 функциях: прямая смена/seek/pause клипа, новая actor
 state, Source helper персонажа, desired-clip/fixed-pose миниигр, retirement,
 entry-latch reset и подтверждённые animation subphase transitions. Последние
 могут выбирать новый клип после общего join, поэтому один поиск следующего JAL
 не заменяет разбор FSM. Включены враги, боссы, NPC, персонажи, эффекты и миниигры.
-37 continue sites: phase/movement/clamp или periodic gates без подтверждённой
 end-branch смены animation owner; не вводить global clamp всем читающим end.
 В частности BroboA1008021DB9C/BroboA1018021E88C после end начинают движение,
 сохраняя тот же live loop. ThunderHatuden285802215A4 считает до7 циклов,
 создаёт discharge на3 и не выбирает новый clip на каждом end.
-1 delegate site внутри resident object getter: не является отдельным owner.

Границы обзора — известные ROM bodies и их прямые API sites. Индивидуальное
прохождение всех врагов/ресурсов и неизвестные косвенные writers не заявляются.

## Исправление

tools/model_transition_patches.py вставляет observer сразу после object getter
8002A3B0. Полный hash его36-byte body и leaf80039544 закрепляет ABI: после
возврата a0 содержит именно выбранный model из actor+74+component*4. Observer
не угадывает component0, не теряет a0/a1 в Source delay slot и не меняет v0.
Он стоит перед другими post-call adapters. Генерация требует полного совпадения
всех363 sites и Source body identities; новый/пропавший query требует разбора.

Общий terminal_model_pose привязывает intent к run/channels/descriptors/duration.
Read-only command_simulation_slice сохраняет actor-local interval через
SimulationSuspension, пока настоящий Source FSM отключает motion scaling.
Возврат RAII восстанавливает предыдущий command view; существующая cancellation
serial проверяется. active_simulation_slice и все movement operands не менялись.
Этот local clock не используется вместо общего world timestamp звука/фейдов.

Native render удерживает duration после forward end либо0 после reverse end
только в том же rational actor tick/denominator. end_direction исключает старую
границу после разворота rate. Source render применяет соответствующее условие
к своему pose-before-advance интервалу. Новый Source tick либо настоящая clip/
seek/pause/clamp/release/heap/reset команда отменяют предыдущий owner/intent.
Переход на другой clip с тем же указателем также имеет другой run.

Guest time/end flags и nondestructive getters продолжают исходную логику.
События не синтезируются, pause/clamp flags не выставляются. Корректируется лишь
временный SDK pose time в свободном frame bank; bone TRS и guest time возвращаются
существующим ModelFrameScope, morph рассчитывается в том же свободном банке.
Paused/zero-rate bone overrides и retirement/barriers сохраняют прежний путь.
Original20/30 не получает Native frame sampling; Manual/RT64 policy не менялась.

## Подготовка и статус

Изменённые native_model_animation.cpp/native_simulation_time.cpp успешно
скомпилированы GCC C++20 -O2 в отдельные objects, без link/исполнения.
Необходимая окончательная CPU generation: startup-model-terminal-after057-v2;
генерация завершилась exit0 с325 post-getter hooks. Все48 затронутых generated C
units скомпилированы GCC C17 syntax-only,0 compilation errors, без link/исполнения.
Log: build-local/model-transition-after057/generation-v2-cli.log; compilation
metadata: generated-c-compilation.json в той же ignored папке.
RSP audio-rsp-v1 не менялся. Переиспользовать старую startup-music-handoff-v1
для следующего EXE нельзя: в ней нет новых325 hooks. Ранний draft v1 сохраняется
в build-local; его receipt больше не соответствует окончательной policy table.
Генерируемый C вручную не правился; ROM/assets и dependencies сохранены.

Новых fixtures/tests/CPU/component/regression/prebuild не было; старые PASS
не перенесены. Windows EXE после057 ещё не собран, игровой результат не подтверждён.
Пачка остаётся открытой: один новый EXE после следующего ответа пользователя
«нет» либо прямой команды собрать. Исправление и документы сохраняются в main/origin.
