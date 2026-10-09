## Уточнение077: в076 fast path для обычного3D был недостижим

При новом статическом разборе обнаружены неверный Z-адрес0038F800 вместо00000400
и отказ на штатной очисткеZ черезcolor image. Историческое описание076 ниже
не доказывает применимость того пути. Исправление и границы выводов:
[performance-depth-readback-after076.md](performance-depth-readback-after076.md).

## После076: пользователь не заметил улучшения; причина остаточной просадки не установлена

Пользователь после проверки076: «ну примерно тоже самое». Предыдущая оценка:
360FPS только в2D меню/маленьких комнатах; обычный мир около80–110FPS,
при дальности1x максимум порядка200+. Новые точные замеры076 не предоставлены.
Отдельный RAM-render вfullSync найден по исходникам, но его устранение в076
не дало заметного пользовательского эффекта. Не считать это доказанной причиной
остаточной просадки и не утверждать, что fast path реально работал в проблемной
сцене:076 без профиля/логов, фактическая eligibility/fallback частота не измерена.
Следующая задача передаётся Астре: независимо локализовать оставшееся узкое место,
проверить применение076 и изучить весь критический graphics/Native путь.
Новых исправлений или сборки при передаче не сделано. Commit/push запрещены
до пользовательской проверки и прямой команды. Все незакоммиченные изменения
main сохранить; не применять reset/clean и упрощения игровой логики.

# После075: отдельный рендер для RAM на каждом Native кадре —076

Пользователь: чистый075 достигает360 только в2D/маленьких комнатах; мир около
80–110FPS, при draw distance1x максимум порядка200+. Диагностическая нагрузка
не объясняет остаток. Последний подробный074 профиль локализовал большой
расход в producer RT64Parse (CPU5–8мс/frame), а GPU timestamp относился лишь
к workload queue, поэтому не учитывал второй GPU проход внутри fullSync.

## Подтверждённый механизм

Frontend оставлял EmulatorConfiguration.framebuffer.renderToRAM=true по
умолчанию. В RT64 State::fullSync это запускает собственный RSP processor,
framebuffer renderer в native resolution, GPU wait, копирование color/depth
в word-swapped RAM и проверкуRAM следующего списка. Затем WorkloadQueue снова
выполняет обычный рендер для показа. Чем больше draw calls, тем дороже оба
набора CPU graphics commands. Это не настройка FPS cap и не медленный AI.

Обратную копию нельзя просто выключить: в ROM подтверждён800271B0, который
после нулевого task count читает противоположный color framebuffer и копирует
его при переходе сцены. На800272B8 загруженa1(source),800272C4 читаетLHU,
800272C8 пишетSH. Существующие verified hooks800272BC/800272F8 ограничивают
только этот call-free участок; связанные размеры читаются из801AA658/801A48C4.

## Изменение

Снимок света каждого опубликованного банка содержит дополнительно захваченный
на guest thread признак Native gameplay и depth address. Producer не читает
позднейшую scene policy другого потока. Новый readback planner допускает только
постоянные screen FB3B5000/3DA800 RGBA16 width320,height<=240 иZ38F800.
Offscreen/FB-copy/reinterpret/unknown workloads сохраняют прежний eager path.
Original, scripted scenes, rawRDP и replay без установленного callback также
используют прежний путь. AI/contact/time/distance/limits не менялись.

Для допущенного Native кадра fullSync загружает данные; WorkloadQueue рисует
игру один раз. RAM-копия отложена до настоящего потребления: при возврате
Native→Original, перед CPU framebuffer copy, изменением producer config,
перед noneligible/raw workload. RAM materialization ждёт уже опубликованные
workload/present IDs, затем под shared workload/manager locks конвертирует
готовые GPU color/depth в оригинальный16-bit word-swapped формат. Нет нового
показа/полного повторного рендера сцены, повторных игровых callbacks или VI.
Hash tracking/rdramCheckPending затем учитывают настоящие CPU writes.

Auto/2x/4x поддержаны: native pixel centers отображаются через фактический
resolutionScale GPU target, включая retained allocation после resize.
Специальный optional high bit вoffset.x и16.16 scale используются только
во время этой materialization. У старого eager path offset.x0: pixel addressing,
color/depth/packed converters и endianness остаются прежними. Dither seeds
планируются в прежнем порядке color→depth; копия сохраняет последнее seed.
Результат масштабированного GPU framebuffer читается вnative320x240 grid.

Адаптированы только build-local State/NativeTarget и framebuffer write shader
copies: sources checked by exact SHA256; dependency checkouts/pins/Dora64 не
редактируются. Все прежние sprite edge shader changes сохранены. CPU generation
прежняя чистая075; generatedC вручную не менялся. Windows076 тоже без логов,
профилировщика, тестов/CPU checks и запуска игры/renderer/GPU агентом.
Доказательства и границы: research/test076-framebuffer-readback-analysis.json.
Компиляция не подтверждает GPU pixels, переходы или конечныйFPS. Git после
пользовательской проверки и прямой команды.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 24.125 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-076.exe.
SHA-256: `3f70a421d6c5cde13590958ceba56fd7aa788984288038e2c7b1707203624d46`.
Все 240 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
FPS измеряет пользователь; автоматические журналы и профилировщик выключены.
Commit/push не выполнялись.
