# Точечная локализация после077

2026-10-08. Пользователь сообщил Native5x/360 около120FPS, гора до70;
начальная катсцена около120, чистые2D экраны360, CPU/GPU примерно10%.
Новый точный профиль пока отсутствует. Низкая суммарная загрузка не доказывает
отсутствие CPU bottleneck: один последовательный поток/ожидания возможны.
077 не считать успешным устранением оставшейся просадки.

## Прочитано статически

Проверены HLE interpreter, State fullSync/checkRDRAM/inspect, RSP processor,
BufferUploader/texture upload waits, native frame gates, общий producer и
workload/present queues. В фактическом Windows build.ninja у RSP/HLE/state,
workload queue и native_gameplay_preview /O2 /Ob1 /DNDEBUG, не Debug.
Inspector при скрытом developer UI завершает только overlay frame;
дорогой полный developer inspector не выполняется. Native RAM bypass077
намеренно ограничен gameplay и не покрывает intro policy; этим не объясняется
вся просадка, доля затрат не измерена. В deferred ветви fullSync остаются
обязательные upload/GPU/texture waits. Удалять их без определения владения
буферами нельзя. Отдельного установленного дефекта после077 пока нет.

## Разрешённый пользователем078

Прямая команда: «Да, сделай точечный замер»; уточнение, что запрет профиля
касался сравнения FPS, а при расследовании можно. Это observation-only build,
не очередной speculative FPS fix. DORA3_GRAPHICS_PROFILE=ON при сохранении
DORA3_DIAGNOSTICS=OFF, DORA3_PROFILE не определён. Clean CPU generation
startup-clean-after074-v1 (diagnostics=false/profiling=false) сохранена.
Флаги включены только для10 инструментированных translation units; generated
C не меняется и не регенерируется. После fix build_windows.ps1 без
-GraphicsProfile явно выставляет OFF, не оставляя старое значение CMake cache.

Отдельный src/graphics_profile сохраняет раз в секунду агрегат:
logs/<run>/state/graphics-profile.jsonl. Нет per-command/actor/vertex timer,
trace registry, PCM журнала или generic игрового профайлера. Native update и
построение кадра имеют только внешние timers. Профиль не требует запуска
агентом; файл пишет EXE при запуске пользователя.

Области: producer, screen (включает ожидание RDRAM mutex), HLE/checkRDRAM,
fullSync/flush/tiles validation/call preparation/draw upload; eager path,
render setup/target safety/command recording/CPU readback; upload/GPU/texture
waits; queue advance/inspector; deferred materialize и ожидание прошлых IDs;
workload/render/upload wait/GPU wait/present/present wait. HLE inclusive может
содержать fullSync; использовать self, не складывать вложенные durations.
Разные потоки перекрываются. GPU timestamp — прежний участок display-render,
не полная GPU стоимость. Thread CPU время Windows грубое, дополнительно
сохраняются thread cycles без перевода в миллисекунды.

Counters: выходные и гостевые кадры, суммы геометрии/fb pairs/tiles, actual
eager/deferred decisions, materializations; причины отказа (request off,
operations/format/height/color/depth/depth-clear/texture-copy/no writes).
Gate reasons снимаются на guest thread для точного bank: build/gameplay,
dimensions/CPU framebuffer mode. Несколько причин могут относиться к кадру;
reject_tile_copy считает tiles, а не кадры. Суммы geometry делить на число
render calls; sample boundaries могут разделять начало/окончание frame.

RT64 меняется только генерируемыми build-local копиями; исходные зависимости
и pins сохранены. Новый interpreter pinned SHA256
1f979869283e3679d4c9214cd747ac2b856fba28142056d43b16d299872109a4.
Существующие077 guards, порядок очередей, waits, dither seeds, gameplay/AI,
контакты, elapsed/catch-up/animation boundaries не меняются.

## Проверка пользователем

Оставить Native/5x/360. Достаточно нескольких секунд на2D экране, начальной
катсцене и20–30сек в тяжёлой сцене (гора). Закрыть игру обычным способом.
Сравнивать stage times/deferred counters, не считать профиль чистымFPS benchmark.
Агент не запускал игру, эмулятор, renderer/GPU, tests или CPU checks.
Компиляция подтверждает только сборку. Commit/push не выполняются.

##078 собрана и упакована

MSVC RelWithDebInfo /O2: exit0,24.0сек после уточнения уникальной границы
генерации CPU readback (первый configure отказал на неоднозначном шаблоне;
исполнение игры/тестов не выполнялось).10 translation units имеют только
DORA3_GRAPHICS_PROFILE; CMake diagnosticsOFF, generic PROFILE отсутствует.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-078.exe
SHA256 59194858c49a10c359ee931298ca3d157f15d1a105390c9ca0acb7775f443045.
Все246 старых файлов вне logs сохранили размер/mtime; общиеDLL совпали по
хэшам упаковщика.077 EXE/PDB/receipt сохранены. Игровых измерений078 нет.
Ждём короткий пользовательский запуск; новые speculative FPS fixes не внесены.
