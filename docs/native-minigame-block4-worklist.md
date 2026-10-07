## После048: Mini8 damage/motion и race/plane cameras — правки ещё не проверены

Terminal pose boundary:38 Source sites для one-shot return/результатов и
аналогичных pause/seek, включая6 character variants; без новой проверки.

Race/plane circular follow/finish подключены во всех4 viewports; зимняя гонка
использует тот же Source helper. Runtime result пока не проверен.

Сохранены Source contactXZ publication, body/shadow world-local coupling и общая
matrix invalidation для reached Derived actor poses. Source draw копирует позиции,
которые затем читает damage713C; прежний Native draw rollback их стирал.
Actual damage/walking/draw interleaving не покрыты старыми isolated/intro cases.
Тесты и новый EXE отложены до «нет» пользователя. Подробности и future cases:
[native-regressions-after048-batches.md](native-regressions-after048-batches.md#пачка5-миниигра8--взрыв-без-урона-и-перемещение-с-source-cadence).

## Test-045: весь блок4 — миниигры и специальные режимы

Завершены девять миниигр и три общих режима: выбор, участники/настройка,
результаты. Windows045 из startup-minigame-block4-v1 сохранён в
D:/Games/DoraRecomp3/testbuild после30 connected CPU checks.
Original20/30 сохраняет input/AI/FSM, счёт/раунды/RNG/waits, sound,
clip/resource/spawn/free. Manual60..360 обновляет реальные уравнения
движения, повороты/масштаб, камеры, модели/эффекты и2D с исходной скоростью.

Движение участников1/4 учитывает промежуточные snapshot/projection fields.
Все три косвенных обработчика8 включены. Terrain query использует настоящую
CPU геометрию и возвращает scratch/stack. Контактные импульсы остаются
однократными. Native draw сохраняет входы actor transforms, производные
матрицы, world echoes и SDK scale globals; следующие Source updates видят
свои значения. Связанные velocity/acceleration и camera moving-target rates
интегрируются по времени; Source end step и дробные partitions согласованы.

CPU доказательства:695 frozen044/current Original instruction streams,
все9 модулей/3 common callback pairs,138 Source/Native cases и946 slices,
20/30Hz и Manual60/73/120/144/165/360, command-once и Source restoration.
Connected suite проверяет прежние объекты/NPC/врагов/оружие/урон/предметы,
камеры/катсцены/UI/модели/эффекты/ввод, переходы, очереди и кадровые банки.
Воспроизводимые cache entries сохраняют исходное execution provenance;
изменённые зависимости запрещают reuse.

2593 описания,2262 смысловых имени функций и5774 scoped variable names
закреплены через ROM-qualified каталог; ABI и canonical C сохранены,
адресные соответствия и документы обновлены. Работа сохраняется в private
main коммитом/push; архивы только дополнительные. Прежние EXE, ROM,
настройки, сохранения и DLL сохранены. Donor и32 dependency pins неизменны.

Первая Windows compile попытка в ограниченной среде завершилась C1902.
Версии компонентов MSVC совпадали; ошибка повторилась на отдельном C файле,
а та же компиляция вне ограниченной среды прошла. Windows045 собран повторно
из того же проверенного кода/генерации; оба журнала сохранены с SHA256.

CPU states/resources и достигнутые ветви синтетические; это не полное
прохождение миниигр и не визуальная проверка. Игра/renderer/GPU локально
не запускались. Общий variable FPS ещё требует блока6 — итоговой интеграции,
переходов, fallback/reset и связей всей игры. Его выбирает пользователь;
автоматически новый блок не начинается.

# Блок4: миниигры и специальные режимы

Реализованы все9 миниигр и3 режима общего модуля: выбор, участники/настройка,
результаты. Точные12 ROM-qualified update/draw pairs зарегистрированы в Native
policy. Source сохраняет20/30Hz input edges/repeat, FSM, счёт, раунды, RNG,
wait, sound, clip/resource/spawn/free. Manual задаёт60..360 обновлений движения,
камер и модельных поз/эффектов. Генератор закрепляет695 Original instruction
streams и конкретные поля/роли/ROM hashes; декомп функции имеют смысловые имена.

Native вычисляет записанные Source уравнения с дробным временем: скорость,
ускорение, trig, подход/затухание, world/local XYZ, Euler/scale, camera parameters.
Контактные импульсы и точечные размещения остаются командами. Grounded участники
используют исходный CPU terrain query с восстановлением scratch/stack. Draw
временно владеет всеми входами transform и производными; после кадра возвращает
Source память. У моделей сохраняются реальные allocation/run/seek guards,
Original sprite кадры/команды; результаты имеют дробные стрелки и opacity.
Выбор/участники используют статические ROM координаты и исходные input/wait
счётчики; переключение рисунка и готовность игроков сохраняют Source длительность.

Общий framebuffer copy относится к startup scene transition800271B0; прежний
drain-before-resource barrier сохраняет владение обоих банков. Он не является
особым callback миниигры8. Три косвенных обработчика8 включены отдельно.

Весь блок4 завершён:30 connected CPU checks PASS, Windows045 собран и
сохранён в общей папке. Прежние proofs сохраняют свои точные входы и исходное
execution provenance. CPU fixture использует synthetic state
и явные command/resource boundaries; все695 Source instruction streams
сравниваются с frozen044. Визуальная проверка и реальное прохождение — у
пользователя, локальная игра/renderer/GPU не запускаются. Блок6 остаётся отдельно.
