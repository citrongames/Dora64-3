## test-093: flower texture lifetime after077

Clean build. Deferred rendering now retains every sampled texture of the current
frame before cache eviction. Check the swaying flowers in Fairy Forest, including
several loop boundaries, and other previously flashing sprites. Visual result is
pending user playcheck. Camera/lava092 and performance improvements retained.
No diagnostic logs/profilers, no agent game or tests. Previous builds preserved.

## test-085: diagnostic post084 measurement

Preserves084 handoff improvement and080 cache. Adds sampled geometry phases and
retains shared-clock frame timeline. Same gameplay, rendering and timing rules.
Run20-30seconds in the same heavy scene, then exit. No additional FPS fix claimed.

## test-084: clean Native scheduler handoff

Explicit delivery of pending graphics/completion events at Native update and
post-submit boundaries. Preserves080 cache, all gameplay and ownership checks.
082 geometry batching remains disabled. Diagnostics/profilers OFF.
Compare the same heavy scene; actual FPS improvement not yet measured.

## test-083: frame critical path measurement

Preserves080 fix;082 geometry batching disabled after no user-visible improvement.
Diagnostic shared-clock frame trace; gameplay and synchronization unchanged.
Play20-30s in the same heavy area, exit and report ready. Old EXEs preserved.

## test-082: clean geometry batching after081

Preserves080 TMEM hash cache. Batches vertex buffer writes and triangle texture
bounds bookkeeping without changing geometry, gameplay, distance or FPS limit.
DIAGNOSTICS/GRAPHICS_PROFILE/generic profiler OFF. Check the same heavy area.
No agent game/tests execution; performance and visual check remains with user.

## test-081: диагностический замер после ускорения080

Сохраняет исправление080. Измеряет остаточные расходы графики и выбранных Native
функций, попадания TMEM memo cache. Generic profiler и обычные diagnostic logs OFF.
Запусти20–30сек прежнюю тяжёлую сцену с теми же настройками, затем закрой игру.
Сборка сама не подтверждает прирост FPS. Старые EXE/ROM/settings/saves сохранены.

# Windows test-080: чистый билд с кэшем результата TMEM hash

Убрано повторное хэширование неизменных палитровых текстур: точное сравнение
всех4096bytes и параметров. Изменившиеся данные используют исходный расчёт.
Gameplay, дальность и GPUcache semantics сохранены. Профайлер и диагностические
журналы выключены. Проверить тот же тяжёлый участок Native5x/360; новыйFPS
пока не измерен. Старые EXE/ROM/settings/saves/DLL сохраняются.

# Windows test-079: уточнение измеренного графического расхода

078 показала ~4мс на обработку текстур и~2.8мс на HLE в тяжёлой сцене.
079 измеряет внутренние функции по выборке1/64, сохраняя алгоритмы078.
Native5x360:15–20сек начальной катсцены и15–20сек тяжёлого уровня, затем закрыть.
Журнал logs/<run>/state/graphics-profile.jsonl. Это диагностика, неFPS fix.

# Windows test-078: точечный профиль графики

Dora64-3-test-078.exe собирает logs/<run>/state/graphics-profile.jsonl.
Это диагностическая сборка для локализации затрат после077, не новыйFPS fix.
Native/5x/360: несколько секунд на2D/начальной катсцене и20–30сек на горе,
затем закрыть игру. Общие trace/audio/runtime logs и игровой profiler выключены.
ROM, settings, saves и общиеDLL сохраняются. Агент игру/тесты не запускает.
Подробности: docs/performance-graphics-profile-after077.md.

# Test-077: исправление применения deferred framebuffer readback

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-077.exe.
Исправлены два отказа fast path076: реальный адрес Z-буфера Doraemon3 и его
очистка через color image. Очистка учитывается как Depth target, которым
пользуется RT64. Игровая логика, дальность, AI, контакты и время сохранены.
Без профайлера и диагностических логов. Тесты и игра агентом не запускались.
Успешная сборка не подтверждает прирост FPS; результат проверяет пользователь.
Старые EXE, ROM, настройки, сейвы и DLL сохранены. Коммит/пуш не выполнялись.
