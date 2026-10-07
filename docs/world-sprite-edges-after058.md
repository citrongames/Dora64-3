# Цветная обводка спрайтов мира после058

2026-10-08. База main e97b7e6; последний пользовательский EXE058.
Пользователь видит обводку у носа/круглого кулака Дораэмона, деревьев и цветов
в3D мире. Запрос исключает2D меню. Текущая пачка остаётся незакоммиченной:
новое прямое правило разрешает commit/push только после пользовательской
проверки результата, а не автоматически после fix/build.

## Подтверждённая причина

Original GIMO material13 выбирает TEX_EDGE через общие80039B7C/39C40/39D04/
39DC8. Variants3078/3048 включают CVG_X_ALPHA+ALPHA_CVG_SEL без FORCE_BL,
включая режимы с depth/fog и без них. Это общий контур моделей/декораций,
не только animated bitmap channels.

RT64 TextureSampler.hlsli фильтровал RGB и alpha независимо: bilinear,
N64 three-point и average. RGBA5551 texels сalpha0 всё ещё хранят RGB,
и этот скрытый RGB участвовал в соседнем видимом filtered sample. Затем
RasterPS отсекает coverage<1/8; оставшиеся TEX_EDGE pixels могут писать
непрозрачный цвет. Поэтому посторонний цвет прозрачного фона создавал обводку.
Полностью прозрачный sample уже отсекается исходной coverage проверкой;
ошибка — подмешивание скрытого цвета в частично покрытый край.

Конкретные ROM assets, визуально прочитанные как исходные изображения:
bank0:0000, component2 — красный нос16x16; component5 — белый кулак16x16.
Оба CI4:108 transparent/148 opaque texels,40 transparent border texels,
все40 имеют RGB, отличный от соседнего видимого цвета. Палитра у прозрачных
texels содержит ненулевой RGB. Аналоги есть у повторных моделей Дораэмона
0479/0970 и других круглых частиц/частей. bank33:00A4 — растительность,
01D4 — белый цветок,0360 — розовые цветы; найдены также трава и кораллы.
Назначение этих изображений определено по исходным ROM texels, не игровым run.

## Исправление

src/shaders/world_sprite_edges.hlsli и tools/prepare_rt64_sprite_edges.py
создают build-local raster sampler. До фильтрации RGB каждого исходного
sample умножается на его alpha; после фильтрации RGB возвращается в straight
форму делением на filtered alpha. Alpha самого результата не меняется.
Таким образом alpha0 texels не окрашивают соседнюю видимую часть. Opaque
samples/nearest sampling, Original combiner, coverage threshold, fog, depth,
геометрия и игровые FPS/анимации остаются исходными.

Ограничение общей ветви: perspective triangles, CVG_X_ALPHA+ALPHA_CVG_SEL,
без FORCE_BL. Именно такой material13 используют перечисленные world sprites.
Текстурные rectangles2D меню, force-blended translucent effects и framebuffer
textures с alpha-as-coverage проходят прежний путь. Replacement mipmaps
оставлены прежними: оригинальные N64 textures не имеют host mipmaps.
Все три режима фильтрации исправлены; bitmap/palette не переписываются,
CI4/CI8/RGBA/IA и dynamic TMEM используют общий sampled color механизм.

research/runtime/CMakeLists.txt подключает адаптер до создания rt64 target.
Он копирует только CMake/RasterPS/TextureSampler/helper в build-local; все
остальные источники/инструменты/headers берутся из закреплённой библиотеки.
Проверяются исходные SHA256 трёх файлов. CMake dependencies включают helper
и generated sampler, поэтому их изменение пересобирает raster shaders.
Копия RasterPS по содержимому исходная; альфа/blender/coverage не изменялись.
DX12 dynamic+library/specialized и Vulkan dynamic+spec/MRT/MS используют этот
общий sampler. Dependency checkout, pins и первый Dora64 не модифицировались.

## Статический охват и ограничения

tools/analyze_world_sprite_edges.py читает все4976 records из трёх Original
SDK42E30 таблиц (banks0/32/33):1870 GIMO и1475 standalone BMP headers.
Прочитаны все4773 material13 TEX_EDGE records и их display lists; остальные
material kinds:1556 opaque11 и743 translucent12. Это структура ROM,
не утверждение runtime достижимости каждого ресурса или прохождения всех сцен.

В TEX_EDGE lists найдены390 mixed-alpha texture references:304 уникальных
локальных texture/palette адреса у253 моделей,101 reference в billboard parts.
У всех390 обнаружены несовпадающие RGB у прозрачных border texels. Отчёт
research/world-sprite-edge-audit.json содержит адреса/форматы/размеры/счётчики,
без извлечённых пикселей.610 references используют external bitmap segments;
их индивидуальная связь с ресурсом не заявляется. Они тоже фильтруются тем же
sampler после реального Source binding. Не вводятся списки исключений по ID,
scene, actor или FPS. Нос/кулак и растительность охвачены одним исправлением.

Разобранным четырём mode emitters даны смысловые имена в decomp-symbols.json;
legacy ABI сохранён, docs/address-map.csv и docs/decomp-symbols.md обновлены
через update_decomp_docs.py --write. Эти новые annotations требуют обновить
canonical CPU generation перед следующим build; игровой код по поведению
не патчится. Текущая canonical generation after057-v2 теперь устарела по
metadata hash, использовать её для упаковки следующего EXE без генерации нельзя.

## Выполненная компиляция

Успешно скомпилированы12 HLSL вариантов Linux DXC:8 SPIR-V (dynamic/spec,
с MS и MRT) и4 DXIL (dynamic/library, с MS). Только compiler outputs,
не исполнение шейдеров/GPU. Local receipt и compiler logs сохранены в
build-local/sprite-edge-analysis/compiled. CMake configure/generate для live
Linux target завершились успешно, до новых decomp annotations; build.ninja
ссылается на adapted sampler/helper во всех raster targets. Capture wrapper
после configure вернул ошибку exit из-за пустой shell variable; CMake log
подтверждает Configuring done/Generating done, сам configure не упал.

Не выполнялись automated tests, CPU/component/regression checks, pre-build,
fixture runs, игра, эмулятор, renderer/GPU или визуальное прохождение.
Полный Windows EXE пока не запрошен и не собирался.058 не содержит эту правку.
Игровой результат ожидает пользовательской проверки следующего билда.
Git commit/push не выполнялись; база main/origin остаётся e97b7e6.

## Windows059 по прямой команде пользователя

Собран MSVC RelWithDebInfo и упакован059 из незакоммиченного snapshot
поверх e97b7e6. EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-059.exe.
SHA256: 7f38fa70ce1f6e3091cf4c8c762576fd87a6b6bec03a0a0d3f4dea9510ccebba. Окончательная CPU generation —
startup-sprite-edges-after058-v1; audio-rsp-v1 прежний. Новые annotations
включены; after057-v2 для059 не переиспользовалась. DXIL library/dynamic и
SPIR-V specialized/dynamic/MRT raster shaders с новым sampler собраны штатным
CMake. Compile/link exit0. Все189 прежних файлов вне logs сохранены.
Отладочные PDB/EXE058 сохранены в staging/build-local/symbols-test058,
прежний staging — sprite-edges-test059-build/sources-before-test059.zip.
No tests, CPU/component/regression/prebuild/game/emulator/renderer/GPU runs.
Успешная компиляция не является игровым подтверждением. Commit/push до
пользовательской проверки НЕ выполнялись. Контекст выше до запроса билда
сохранён как история; текущая сборка059 уже содержит исправление.

## Пользовательское подтверждение и сохранение этапа059

Пользователь сообщил: «пофикшено, коммит и пуш делай». Исправление сообщённой
цветной обводки спрайтов мира в059 подтверждено его игровой проверкой; он
прямо разрешил сохранить этот этап коммитом в main и отправить в origin.
Конкретные сцены/FPS и полный обход всех моделей этим сообщением не заданы.
EXE059 и compile/package receipts сохраняются как исходные записи сборки;
QA записана отдельно в research/windows-test059-user-confirmation.json.
Новых локальных тестов, запуска игры/renderer/GPU или сборки не было.
Для следующих исправлений продолжает действовать Git только после проверки.
