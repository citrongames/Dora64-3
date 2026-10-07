# Вылет при переходе к катсцене Сидзуки

2026-10-07. Пользователь нажал кнопку рядом с Сидзукой в051 и получил вылет
вместо катсцены. Уточнил, что дефект существовал и до аудиоправки051.
Исходное состояние main c13de4c, рабочее дерево чистое; последний EXE051.

## Доказательства из пользовательского запуска

runtime.log: D:/Games/DoraRecomp3/testbuild/logs/1791382479574/runtime.log.
Последний новый ENTER:8022DA68, section_rom00132F80 (.level1_slot1),
функция dora3_city_kind9_contact_begin_fade_delay_8022DA68. Её contact mode4
блокирует world/player, снимает pause с текущего actor и вызывает резидентные
fade_color_step_set80028878, fade_begin800288EC; далее ставит actor state2.

Windows Event1000:20:17:23,051 PID28448, exceptionC0000409, ucrtbase abort;
Event1001 FAST_FAIL_FATAL_APP_EXIT subtype7. Дамп:
C:/Users/black/AppData/Local/CrashDumps/Dora64-3-test-051.exe.28448.dmp.
Прочитан offline, без запуска игры. PDB GUID c7ce0955-18af-401d-9196-cb8ebf66de7d,
age69 совпал с EXE. Цепочка: native_game_message -> FrameBridge::dispatch ->
Native session update/synchronize -> FadeTimeline::sample(Nanoseconds) ->
FadeTimeline::sample_at -> CxxThrowException -> terminate -> abort.
RVA75AD18 соответствует sample_at+148; текст исключения из точной throw ветви:
«Fade clock moved backwards or invalid fraction».

Сохранённые поля FadeTimeline: previous_ns=0x2A26CAD87E60, previous_fraction=0;
переданный now=0x2A26CABD5158, fraction=0. Разница1781000нс (~1.78мс).
Это фактическое движение командного таймера вперёд за текущий кадр, а не
догадка по последней функции в runtime.log. Ошибки SDL/audio queue нет.

## Причина и исправление

native_gameplay_preview_update фиксирует previous=now в начале обновления.
GameplaySliceScope хранит общий игровой интервал. PlayerInvocation сохраняет
Source command stamp, затем dispatch_native_npc_action ставит
SimulationSuspension: поиск контакта и однократный NPC callback должны
выполняться с Original semantics, без дробного масштабирования движения.

Прежний native_gameplay_preview_command_time использовал gameplay_slice только
при object_scope, иначе читал active_simulation_slice. В suspended NPC path
тот пуст; fallback сохранённого PlayerInvocation был разрешён лишь после
смены epoch. Обычный NPC callback без смены epoch терял timestamp. Хук фейда
выбирал monotonic_time из середины CPU callback, опережая previous текущего
кадра. После блокировки управления synchronize сохранял fade на render_time,
то есть на более раннем timestamp, и строгая проверка выбрасывала исключение.

Теперь командный clock всегда предпочитает существующий gameplay_slice.
SimulationSuspension отключает только motion; мировой command stamp продолжает
действовать. При отсутствии игровой timeline прежний fallback и epoch/reset
правила сохранены. Время остаётся точным SourceTimestamp с долями1/60нс.
Проверка обратного хода в FadeTimeline не отключена, alpha и длительности
фейдов не зажаты искусственно, Original callbacks не ускорены и не повторены.

Аналогичные paths: временно suspended Original geometry/contact callbacks,
инициализация новых объектов в continuous update и projectile local scopes.
Их команды используют тот же общий мировой интервал, а не локальный возраст
actor/projectile. Original режим без gameplay timeline остаётся прежним.
Правка находится в src/native_gameplay_preview.cpp; ABI и ROM bodies не менялись,
генерируемый код и зависимости не редактировались.

## Статус

Правка4be12e2 включена в Windows052 по прямой команде пользователя.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-052.exe.
SHA256: fa270a9cb60b3bfedaa38f0de2abc0e82cf4c52492a7777a8f97353666aad21e.
Компиляция MSVC RelWithDebInfo успешна;168 прежних файлов вне logs не изменились.
CPU/RSP generation051 сохранена с неизменными входами; изменённый C++ пересобран.
Тесты, игра, эмулятор, renderer/GPU не запускались. Пользователь подтвердил исправление катсцены в052.
PDB051 сохранён в Windows staging/build-local/symbols-test051 для будущего анализа.
