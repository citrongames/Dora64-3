# После076: почему deferred readback не применялся к 3D

2026-10-08. HEAD a9a8d2c, существующее большое dirty дерево сохранено.
Пользователь:076 примерно как075; Manual360 даёт360 в2D/маленьких комнатах,
в мире80–110, при рендере1x/дальности1x всё равно ниже300. Это наблюдения,
не новый точный профиль. Коммит/пуш запрещены до его проверки и команды.

## Установленные дефекты076

1. Original80031588/8003158C формирует80000400,800315C8 сохраняет этот адрес
   в801AA650. Он не перенесён distance patches или другим host adapter.
   Физический Z-буфер00000400..00025C00, затем начинается resident code.
   Frame light snapshot правильно читает801AA650;076 ошибочно сравнивает
   результат с0038F800. Любой pair сdepthRead/depthWrite отклоняется.
2. Общий world draw801BB848 вызывает8002F594 на801BB868. Depth-clear builder
   пишет SetDepthImage и SetColorImage с одним адресом Z, затем FillRect;
   8002F6A4 возвращает настоящий экранный color image.076 разрешает только
   color addresses003B5000/003DA800, поэтому отвергает саму очистку Z.
3. RT64 State::submitFramebufferPair уже распознаёт предшествующий fill-only
   pair какfastPaths.clearDepthOnly. WorkloadQueue::getTargetsFromPair для
   него создаёт/использует Depth target поcolorImage.address, безColor target.
   Просто добавить адрес Z в список допустимых color images нельзя: последующий
   readback обратился бы к отсутствующему Color target.

Это статическое доказательство отказа eligibility для штатного 3D-пути, а не
измерение частоты исполнения076. Объясняет, почему прежнее изменение не могло
убрать eager проход таких кадров. Конкретная доля каждого внутреннего этапа
в профиле074 остаётся неизвестной; обещание300/360FPS из этих данных не следует.
ROM hashes/адреса — research/test077-depth-readback-analysis.json.

## Исправление

Общий planner использует проверенный адрес00000400 и допускает только
распознанную RT64 fill-only очистку того же захваченного Z-буфера. Её результат
учитывается какDepth при отложенном чтении. Дальнейшие записи глубины объединяются
с очисткой в одну последнюю копию по существующему механизму. Порядок dither seeds
сохраняется: очистка, color, depth. Не добавляется повторный игровой update/render.

Все прежние guards операций framebuffer/texture reinterpretation, размеров,
Native gameplay и Original/raw fallback сохранены. Materialization по-прежнему
ждёт опубликованные workload/present IDs и GPU completion; CPU framebuffer copy,
Native handoff и смена конфигурации сохраняют обязательное восстановление RAM.
Масштабированное чтение из076 не меняется. Pending Z копируется из того же
Depth target, куда штатный WorkloadQueue выполнил очистку и геометрию.

## Обход критического пути и границы

Повторно прочитан пользовательский074 performance.jsonl: RT64Parse охватывает
HLE/fullSync/загрузку текстур/ожидания. В сценах165/169/150 его thread CPU
около8.16/5.19/6.90мс на кадр. WorkloadQueue render и present выполняются в
других потоках; вложенные значения не складывались. GPU timestamp измеряет
только display queue участок. Низкий resolution scale не устраняет producer
command recording, отдельный RAM-render, конвертацию/readback и fence waits.

Просмотрены producer send_dl, frame-light capture, fullSync, framebuffer-pair
submission, eager/display target creation, RSP upload, native timelines,
frame publication, RAM-copy barriers и очереди. Проверяемый дефект локализован
в eligibility076; нет основания менять HLE, AI, контакты, catch-up, boundaries,
Native callbacks или синхронизацию очередей наугад. Другие их расходы остаются.
Новые внутренние замеры без разрешённого профиля недоступны; в077 профайлер
и диагностические журналы остаются выключенными.

Изменён только src/framebuffer_readback.cpp среди исполняемых исходников;
ROM, generated C, pinned dependencies, sprite shaders, дальность и остальные
незакоммиченные fixes сохранены. Тесты/CPU checks/game/emulator/renderer/GPU
агентом не запускались. После исправления — инкрементальный077, без Git.

## Сборка077 завершена

Инкрементальный MSVC RelWithDebInfo configure/compile/link: exit0,17.375с.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-077.exe.
SHA256: 165c529caff91d7dbf083fe0c49d996cb5d4e135bccff34cf3db0c8e54d4948c.
DORA3_DIAGNOSTICS=OFF, clean CPU generation diagnostics=false/profiling=false;
DORA3_PROFILE отсутствует в compile definitions. Все243 старых файла вне logs
сохранили size/mtime; общиеDLL совпали по хэшам упаковщика. ROM/settings/saves
не переписывались. Старые076 EXE/PDB/receipt сохранены. Новых игровых измерений
нет; успешная компиляция не подтверждает FPS. Commit/push не выполнялись.
