## test141: рост огня статуи TwinFace на Manual FPS

Статуя327 создаёт огонь328 через8022DF9C, overlay125180. ROM размещает
статуи в сценах165 (1) и276 (3); исправление не привязано к сцене.
Огонь начинает со scale0.2, пять Source-шагов приближается к2, затем
растёт линейно. Эти уравнения уже присутствовали в Native-адаптере.

Общий draw при physicalCA вызывает801E8ECC:80031B38 записывает направление
на камеру в actor+C/10. После native_enemy_motion_end менялся yaw actor10,
поэтому следующий дробный шаг отвергал unchanged и удалял уравнения роста.
ModelFrameScope сохраняет кости, но не эти поля actor. Это отдельный путь
от camera-facing префикса KaenHousha, исправленного в test125.

ROM-qualified wrapper801E8ECC отмечает только собственные визуальные записи
в guard живого Native-периода, предварительно проверив unchanged/identity.
Исходный draw выполняется полностью; реальный урон, движение и команды
продолжают инвалидировать период. Рост, скорость, момент рождения, коллизии
и Original-путь не изменены. Общий helper охватывает такие эффекты во всех
локациях; research/twinface-fire-after140.json.

Генерация startup-fire141-v1; Windows Release test141 без журналов,
инкрементально. Игра, тесты, CPU checks и GPU не запускались.
Проверка огня в игре ожидается от владельца; компиляция её не заменяет.
Коммит/push ожидают отдельной команды. Архивы не обновлены.

# TwinFace327–329

Внесены полные closure родителя, fire и moon: progress/reward/contact routing,
sector/cadence births, linked charge/release, наведение по yaw/pitch, collision,
growth/shake/fade и retirement. 16 функций получают semantic names/comments/scopes.

Луна повторяет только реальные matrix position/yaw/roll во время phase34=0.
Exact boundary загружает настоящий parent7C в a0 и пропускает entry/opacity math,
release/clip/sound/flight commands. Parent child0 и child parent/lifetime guarded.
Огонь сначала exponential scale к2, затем linear growth и bounded pitch3.
Death327 Y±10 продолжает настоящую alternating recurrence, alpha новое опубликовано.

Common reward/effect исходящие связи остаются в аудите зависимостей. Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
