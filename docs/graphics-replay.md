# Первая графическая задача: RT64 replay

## Результат

Native runtime дошёл до первой graphics task. Снимок снят в scheduler 80026E24
до osSpTaskLoad/StartGo и SP completion. Отдельный executable инициализировал
исправленный RT64/Vulkan и обработал исходную display list. Устройство:
Microsoft Direct3D12 (RTX2060) через dzn/WSL2. Определён GBIUCode=7: F3DEX2.

RT64 сообщил отсутствие dual-source blending и выбор существующего обхода
через coverage attachment. Подтверждён выбор ветви, не качество blending
на игровых сценах. Библиотеки не изменялись.

| OSTask поле | Значение hex |
|---|---|
| Type | 1 |
| Text / declared size | 80063510 / 1000 |
| Data / declared size | 80078E00 / 800 |
| Display list / bytes | 80120638 / 30 |
| OSScTask framebuffer | 803B5000 |

Текст и data в снимке сравниваются с ROM перед replay. Declared 1000 не является
проверенной длиной инструкций микрокода. Первая задача с вложенными списками
содержит 65 команд настройки состояния: нет vertex/triangle/fill/texture rectangle.
Поэтому успешный разбор не означает появления картинки. Replay сохраняет
исходные VI до completion, не имитирует swap buffer/VI unblack и не продолжает игру.

## Воспроизведение

После генерации CPU/RSP и сборки startup (reproduction.md), из корня проекта
в Linux x86-64, с новыми именами каталогов:

~~~bash
DORA3_CAPTURE_GRAPHICS=1 python3 tools/run_startup.py --rom "$ROM" \
  --executable build-local/runtime-new/dora3_startup \
  --run-dir build-local/run-capture-new

cmake -S research/graphics-replay -B build-local/graphics-replay-new -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-local/graphics-replay-new -j6

python3 tools/run_graphics_replay.py --rom "$ROM" \
  --snapshot build-local/run-capture-new/state/first-graphics.snapshot \
  --executable build-local/graphics-replay-new/dora3_graphics_replay \
  --run-dir build-local/replay-new
~~~

Нужны SDL2 development files, Vulkan loader/driver и графическая сессия (WSLg).
Используются изолированные pinned libraries. Replay открывает исследовательское
окно; runner ограничивает работу 180 секундами, сохраняет журнал/хэши/результат.
Ожидаются SETUP result=0, GBI=7, DISPLAY LIST PARSED, exit0.
Logs/cache лежат в новом run-dir/rt64, не в каталоге Dora64.

## Приватный снимок

140 байт заголовка: 35 little-endian uint32; затем 4 МиБ word-swapped RDRAM.
Заголовок: magic44335231/version1/размер/marker01020304, 16 OSTask words,
14 VI words, framebuffer pointer из OSScTask.
Replay выделяет 8 МиБ backing storage для RT64; верхняя половина нулевая.
Игровой CPU в replay не исполняется.

Снимок содержит игровые код/ресурсы: только ignored build-local. Расширение
snapshot дополнительно исключено в .gitignore. В Git идут лишь инструмент,
адреса и hashes/result. Не распространять snapshot вместе с портом.

VI копируются в update_screen на владеющем ими графическом потоке, затем
передаются scheduler через отдельный mutex. Это последняя опубликованная VI
копия до запуска первой задачи, не одновременный снимок всех аппаратных регистров.
Старый v18/replay-v1 заменён результатами v20/replay-v3 после исправления R012.

Захват RDRAM защищён graphics_rdram_mutex и разрешён только для первой задачи:
инициализационные PI DMA завершены, scheduler освободил SP.
Это не готовая схема снимков всех последующих кадров/оверлеев. После SP
completion CPU может переиспользовать буферы; перенести send_dl в live renderer
без отдельного анализа памяти недостаточно.

Дальше: владение памятью task/display lists/матриц/текстур третьей игры,
интеграция с живым renderer и исходными VI swap/completion, загрузка сцены.
