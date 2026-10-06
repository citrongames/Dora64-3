## Test-046: блок6 интеграции завершён

Windows046 из startup-integration-block6-v1 сохранён в общей testbuild папке.
30 завершённых CPU checks PASS, включая49 сценариев реального общего цикла.
Долгий full-player прерван по прямой команде пользователя; текущего PASS
полного персонажа/NPC matrix нет. Общий pre-build завершён частично.
Native часы запускаются после завершения старого host read;
сессия закреплена за callbacks/ROM/divider/verified reload generation.
Один timestamp связывает движение, камеры, модельные позы, UI и fade.
При выходе сохраняются дробная фаза моделей и исходные дискретные Source
решения/cues без повторного исполнения команд. Удержание двух банков,
stale wakes, смена частоты/модуля, pause/seek/rebind/reset проверены CPU.
Резервные player states19/23/26/30 больше не прерывают Native мир.
Pak/file UI получил Native easing курсора/панелей и hint offset;
device/FSM/I/O и authored blink/sprite остаются Source once.

Имена и ROM-backed описания закреплены в каталоге и документах; старые ABI
и canonical C сохранены. Exact-input proofs сохраняют исходное execution
provenance. Игра/renderer/GPU локально не запускались; синтетические CPU
inputs не заменяют полное прохождение или визуальную проверку.
Четыре auxiliary startup/resource/model control пары и unsafe/unknown/
loading/framebuffer границы оставлены на явном Original fallback;
их Native движение не заявляется. Подробности — native-framerate.md.
Прежние EXE, ROM, settings, saves и DLL сохранены; donor и32 dependency
pins неизменны. Результат сохраняется commit/push в private main.

# Блок6: итоговая интеграция FPS

База: main3d9766f, Windows045; пользователь выбрал весь блок6.

- Запуск часов gameplay/cutscene/minigames/UI/models после настоящего
  Original→Acquiring→Native handoff, без времени ожидания старого кадра.
- Точная принадлежность сессии callbacks/ROM/divider; отмена старых wakes
  при смене сцены/режима и сохранение занятых банков до обоих completions.
- Возврат Manual→Original/Display, pause/resume и resource/runtime reset:
  Source команды/события/input не теряются и не повторяются.
- Общий момент времени движения/камер/моделей/2D в каждом кадре.
- Покрытие игровых режимов и явные причины fallback; безопасность
  framebuffer copy, deferred FIFO, модельных/actor allocation lifetimes.
- Смысловые имена и документы, затем полный connected CPU pre-build,
  один Windows046, проверка сохранности файлов и commit/push main.

Код выбранного блока6 завершён;30 CPU checks PASS, full-player прерван
по команде пользователя. Windows046 сохранён. Общий pre-build частичный.
Точные границы Source fallback перечислены в native-framerate.md.

Не выдавать старые proofs за новый прогон или CPU fixtures за визуальное
прохождение игры. Локальный game/renderer/GPU запрещён.
