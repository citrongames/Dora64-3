## HP backend065 (2026-10-08)

Интерфейс и семь item switches064 прежние. Backend Infinite HP исправлен:
адрес Original maximum правильно расширяется со знаком, progression flag
не блокирует восстановление выбранного HP. Windows065 собран без тестов и
игры; игровой результат ожидает пользователя. [Разбор](hp-cheat-after064.md).

## Читы после063 (2026-10-08)

Добавлена вкладка Cheats после Controls, как у Dora64, и индикатор Cheats On
справа внизу. Восемь сохраняемых независимых опций: полный HP текущего персонажа
по его оригинальному максимуму и99 с заморозкой для семи указанных предметов
(IDs1/2/3/4/5/8/11). Backend game-thread-only; exact-ROM store hook до проверки
смерти и common add/get hooks до расхода. Original/Display/Native/Manual,
reset/input/cutscene fixes060..063 сохранены. Подробности: [cheats.md](cheats.md).
Test064 собран по прямой инструкции пользователя; compilation/package exit0.
Тесты/игра/renderer/GPU не запускались; commit/push ждут его проверки и команды.

## Reset game (2026-10-08, после059)

Реализован Reset game между Continue и Exit game, с тем же центральным
подтверждением, текстами и размерами, что в Dora64. Внутренняя сессия игры
запускается заново без пересоздания окна/приложения, с сохранением настроек
и EEPROM; несохранённый прогресс теряется. Original и Native/Manual используют
общую ROM-подтверждённую границу. Подробности: [menu-reset.md](menu-reset.md).
Windows EXE ещё не собран, игровая проверка не выполнялась. Упоминания
отсутствующего Reset ниже относятся к историческим сборкам022–024.

# Port menu и управление — пункты 2 и 3 плана на время прохождения

Это план, согласованный в чате после аудита памяти, а не нумерация раннего
`roadmap.md`. Пункт 1 пользователь считает достаточно проработанным для перехода
к функциям порта. Полнота аудита всех ресурсов и GPU этим не утверждается.

## Test-024 — Graphics и оформление из Dora64

Test-023 пользователь подтвердил рабочим. По его следующему запросу перенесены
общие элементы Graphics из Dora64680d81f, с отдельным уточнением: Manual FPS
должен применяться даже выше частоты монитора.

- Monitor: имена и режимы SDL, выбор монитора переносит окно; выбранному монитору
  назначается desktop-размер, как в Dora64. Отключённый индекс ограничивается
  доступными дисплеями. Refresh обновляет список через запрос на основной SDL-поток;
  SDL_DISPLAYEVENT также запрашивает обновление.
- Output resolution: стандартные оконные размеры плюс поддерживаемые размеры
  выбранного монитора, сортировка и удаление повторов разных Hz. Borderless
  показывает desktop/native размер и Hz, не меняет видеорежим ОС.
- Frame rate: Original / Display / Manual; ручной предел60–360FPS, исходное
  значение144. Показаны Display Hz и Target FPS. Modern включает Display,
  Original возвращает Original. Старые настройки без этих полей остаются Original.
- Manual намеренно отличается от Dora64: RT64 больше не ограничивает targetRate
  частотой дисплея. При60Hz и Manual240 целевое значение остаётся240. Подпись:
  Smoother motion with the original game speed. Display follows the display refresh
  rate; Manual can exceed it. Это целевое, а не гарантированное число кадров.
- Общие цвета/размеры взяты из исходника меню: title background, приглушённые
  пояснения Controls, default item width, popup buttons120/100, table252, окно760×500,
  padding22×18, поля48 с масштабированием от720p, footer54. Убрана собственная
  раскладка desktop-вкладок через child-окна. Footer содержит только существующие
  Continue/Exit; Reset game пока не реализован. FPS получает то же сглаживание
  показаний25%/75%, что в Dora64. Текстура шрифта и базовая тема те же из pinned RT64.

Настройки сохраняются в прежнем port_pc_settings.json: graphics.display_index,
display_width/height/refresh_rate, frame_rate_mode (Original/Display/Manual),
frame_rate_limit. Общие сохранения и bindings не меняются.

### Источники и границы переноса

| Источник | Переиспользование |
|---|---|
| Dora64/src/system_overlay.cpp @680d81f | refreshDisplays, displayCombo, outputModeCombo; drawGraphicsTab, drawMenu, drawControlsTab, drawFps, ScopedOverlayScale |
| Dora64/src/rt64_render_context.cpp @680d81f | соответствие режимов RefreshRate и refreshRateTarget |
| pinned RT64/src/hle/rt64_application_window.cpp | setDisplayConfig, включая восстановление Win32 chrome и доступность заголовка большого окна |
| pinned RT64/src/hle/rt64_workload_queue.cpp | реальный выбор targetRate; снят только clamp Manual до swapChainRate |

Часть SDL адаптирована к поточной схеме Dora64-3: меню читает копию каталога,
окно меняет только SDL-поток. RT64 userConfig меняет его producer-поток;
swapChainRate читается под configurationMutex и публикуется атомарно.
Перетаскивание/изменение размеров окна не отменяется каждый кадр.

Снятие ограничения RT64 выполняет tools/prepare_rt64_framerate.py: проверяется
SHA256 pinned исходника, создаётся только build-local копия. CMake заменяет ровно
эти два translation units; библиотеки, pins и Dora64 остаются неизменными.

При Manual выше Hz VSync отключается на present-потоке перед проверкой resize;
при Original/Display и Manual≤Hz возвращается. Это снимает дополнительное
ожидание монитора: одной правки targetRate было бы недостаточно. Vulkan использует
IMMEDIATE, если backend его поддерживает; ограничения драйвера/композитора не
обходятся. D3D12 получает syncInterval0 и ALLOW_TEARING при поддержке.

Игровые адреса/хуки, CPU/RSP генерация, VI/scheduler и звук не менялись.
Высокая частота использует штатное frame matching/interpolation RT64. Качество
движения/стыков сцен третьей части не подтверждается проверками без GPU:
это отдельная визуальная проверка пользователя, Original остаётся доступным.

Проверки: JSON roundtrip/profiles, CPU ImGui geometry/grey help/Graphics modes,
SDL display wrappers (два монитора, dedup, Refresh, fullscreen/failure, disconnect,
отсутствие повторного resize), ASan/TSan, компиляция и запуск извлечённого
реального switch RT64 (Manual240@60Hz, Manual360@165Hz, Display, Original),
SDL virtual-controller/audio regression, сборки Windows/Linux. Никаких игровых
окон, рендера или реального переключения мониторов агент не запускает.

Справка SDL: https://wiki.libsdl.org/SDL2/SDL_GetDisplayMode,
https://wiki.libsdl.org/SDL2/SDL_SetWindowPosition,
https://wiki.libsdl.org/SDL2/SDL_DisplayEvent.

## Test-023 — фоновый ввод геймпада

Удалён добавленный агентом Menu scale. Масштабирование только автоматическое,
по высоте окна относительно720p, как в Dora64. Старое поле menu_scale игнорируется
и удаляется из port_pc_settings.json при загрузке; остальные настройки сохраняются.

В Controls возвращён Background gamepad input с подписью Gamepad only; keyboard
input still requires focus. Опция сохраняется в port_input_settings.json, ключ
background_gamepad_input. На чистом профиле включена, как в Dora64; существующий
сохранённый выбор имеет приоритет. SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS
применяется до SDL_Init и при изменении опции на основном SDL-потоке.
Клавиатура без фокуса не управляет ни игрой, ни меню. Зажатая клавиша после
возврата фокуса требует отпускания. При отключённой опции фоновый геймпад тоже
обнуляется. Меню забирает игровой ввод в обоих режимах; Back и навигация
геймпадом доступны также без фокуса при включённой опции.

SDL dummy/virtual controller проверяет on/off, реальное значение SDL hint,
одновременные клавиатуру/геймпад вне фокуса, возврат фокуса и закрытие меню
с зажатой кнопкой. JSON проверяет сохранение опции и удаление старого масштаба.
Windows/Linux frontend компилируются; реальную игру/устройство проверяет пользователь.

По отдельному запросу пользователя работа консолидируется в main приватного
Dora64-3. Обе дополнительные ветки — предки текущей истории, объединение
fast-forward сохраняет все коммиты; после push main удаляются только эти refs.

Ниже — история согласования меню.

## Test-022 — соответствие Dora64

Test-021 пользователь проверил: «вроде работает», затем уточнил соответствие
первой части. Меню теперь повторяет оформление и порядок готовых разделов:
Game / Graphics / Audio / Controls. Названия: Settings profiles, Show FPS counter,
Display mode, Output resolution, Game render resolution, Texture filtering,
Master volume, Stick dead zone, Restore default controls.
Переназначение — три колонки Action/Keyboard/Gamepad и окно Bind control с
Clear binding/Cancel. Подвал: Continue, Exit game → Exit game? → Exit/Cancel.
Нереализованные Cheats, камера и Reset game не добавлены как пустые элементы.

F1 меню не вызывает. Открытие: Escape / Back–Select.
FPS использует параметры Dora64: текст (0.72, 1, 0.76, 1), фон (0.025, 0.03, 0.04),
alpha 0.72, отступ12, padding8×5, rounding5, масштаб относительно высоты720p.
Цвет проверяется по CPU vertices ImGui без графического вывода.

Оконный режим по умолчанию 1280×720. Output resolution содержит стандартные
размеры Dora64: 640×480, 800×600, 960×540, 1024×768, 1280×720, 1600×900,
1920×1080, 2560×1440, 3840×2160. Сохранённый пользовательский режим имеет
приоритет при запуске. Обычное перетаскивание края окна не сбрасывается каждый кадр.
Полноэкранный режим использует разрешение desktop/native.
Exit game передаёт запрос на основной поток и вызывает обычный ultramodern::quit,
как закрытие окна; принудительного _Exit для этого нет.

Файлы с префиксом port, как уточнил пользователь:
- port_pc_settings.json: show_fps, master_volume0–1, graphics и menu_scale.
- port_input_settings.json: bindings/action/keyboard/gamepad, stick_dead_zone_percent.

Общие поля и GamepadInput serialization соответствуют Dora64, версии14/3.
Идентификаторы действий третьей игры независимы от игровых названий первой.
Дополнительные кнопки SDL используют диапазон1000+, стандартные — номера Dora64.
Неизвестные JSON-поля сохраняются, неподдержанные функции не активируются.
Windows: рядом с EXE/ROM. Linux live: рядом с выбранным ROM, как в Dora64.
Диагностика остаётся отдельно; saves упаковка не заменяет.

Миграция читает port-settings.json только для отсутствующих новых файлов;
старый файл остаётся на месте. Новые файлы при повторном запуске имеют приоритет.
Нетронутая стандартная раскладка test-021 заменяется на Dora64, пользовательские
переназначения сохраняются. Упаковка настройки не создаёт и не перезаписывает.

Стандартная клавиатура: WASD — stick, Space/C/Z — N64 A/B/Z, Enter — Start,
Q/E — L/R, arrows — D-pad, IJKL — C-buttons. Геймпад: left stick, A/B, LT, Start,
LB/RB, D-pad, right stick. Dead zone25%. Restore default controls возвращает
эту раскладку. Названия Actions оставлены N64 по просьбе пользователя.

WSL: до SDL_Init включается SDL_HINT_LINUX_JOYSTICK_CLASSIC при наличии WSL_INTEROP
или WSL_DISTRO_NAME, точно как в Dora64/main.cpp. Обычный Linux/Windows сохраняют
обычный backend. Реальное устройство под WSL агентом не запускалось.

Проверки без игры/GPU: JSON schema/roundtrip/migration, default720p, F1 inert,
quit request, четырёхвкладочное меню/навигация/зелёные FPS vertices ImGui,
SDL virtual pad и стандартные bindings. Windows/Linux RT64 компилируются.
Живое изображение/полноэкранный переход и прохождение проверяет пользователь.

## Реализованный набор

| Возможность | Реализация |
|---|---|
| Открытие меню | Esc; Back, View или Select первого SDL GameController |
| Навигация | Мышь; Tab/Shift+Tab, стрелки, Enter/пробел; крестовина/левый стик, A, B |
| Размер | Окно изменяет размер, меню вписывается в обе оси; масштабирование по высоте720p как в Dora64; без ручного масштаба |
| Звук | Общая громкость 0–100%, включая mute, с сохранением AI timing и числа PCM frames |
| Экран | Оконный и borderless fullscreen, переключение на SDL-потоке |
| Рендер | 1x, 2x, 4x, целочисленный масштаб под окно; N64 three-point либо bilinear |
| Профили | Original = 1x + three-point; Modern = масштаб под окно + bilinear |
| FPS | Счётчик кадров вывода через RT64 present draw hook; флажок Show FPS counter сохраняется |
| Переназначение | 18 действий N64: 14 кнопок и четыре направления стика; отдельно клавиатура и геймпад |
| Оси | Любое направление обоих стиков и триггеры можно назначить на действие; аналоговая величина для N64 stick сохраняется |
| Конфликты | Назначения одной физической кнопки меняются местами, не дублируют два действия |
| Очистка/сброс | Clear binding в окне Bind control снимает привязку; Restore default controls восстанавливает исходную раскладку и dead zone  |
| Мёртвая зона | 5–50%; одинаковая обработка осей в игре и меню |
| Подключение | Автоматическое подключение первого подходящего контроллера, закрытие отключённого, повторное подключение; имя отображается в меню |
| Фокус | Вне активного окна N64 получает нулевой ввод; возврат удержанных кнопок требует отпускания |
| Изоляция меню | Игровые кнопки/стик обнулены, пока открыто меню; после закрытия удержанные кнопки не проходят в игру |
| Настройки | `port_pc_settings.json` и `port_input_settings.json` рядом с ROM/EXE; атомарная замена, проверка типов/диапазонов, сохранение повреждённого файла в `.invalid` |

Профили не меняют управление, громкость и режим окна. Формат 4:3, камера и
скорость игры прежние. Широкий экран, интерполяция, новая камера, Android touch,
локализация и дополнительные игровые функции — отдельные следующие пункты.
Интерфейс меню сейчас на английском, как общая основа меню до локализации.

**Игра продолжает идти при открытом Port menu.** Это не пауза игрового scheduler.
Для настройки в опасном месте сначала используй оригинальную паузу игры.

В Controls выбери привязку, отпусти кнопку выбора, затем нажми новую клавишу,
кнопку или отклони стик. Esc / Back отменяют захват. Эти средства
открытия меню зарезервированы и не могут быть назначены игровому действию.
Небольшой дрейф стика не блокирует доступ к меню настройки dead zone.

## Разделение потоков и переиспользование Dora64

За основу взяты общие интерфейсы закреплённого Dora64 `680d81f`: RT64
`ApplicationConfiguration::updateOverlayInput/drawOverlay`, ImGui навигация,
масштабирование с восстановлением общего стиля, present draw hook для FPS,
настройки resolution/threePointFiltering. Зависимости и сам Dora64 не менялись.

`port_settings.cpp` хранит валидированный снимок под mutex. Разные группы
настроек обновляются отдельно, чтобы захват кнопки не терялся из-за изменения
громкости. Windows заменяет файл через MoveFileExW; Linux через rename.
О проблеме сохранения есть сообщение в меню и stderr; ошибка не аварийная.

`live_io.cpp` читает SDL только на основном потоке, публикует копию физического
ввода. `port_input.cpp` обрабатывает фронты нажатий, захват и отпускание.
RT64 читает копии, не трогает SDL_GameController handle. Меню и аудио не меняют
память N64. Громкость применяется при постановке нового PCM, порядок L/R прежний.

Настройки графики применяет поток производителя RT64 на ближайшем update_screen,
через updateUserConfig. Полноэкранный режим меняет основной SDL-поток. ImGui
используется только внутри существующих блокировок inspector. FPS — атомарный
счётчик вызовов draw hook; это частота вывода, не частота логики игры.

Новых адресов, guest hooks, SDK replacements, DL/matrix patches и изменений
генерации CPU/RSP нет. Исправление startup из test-020 сохранено.

## Проверка без игры

`python3 tools/check_live_io.py` проверяет настоящий адаптер через SDL dummy и
virtual controller: исходные кнопки, нормализацию диагонали, focus, dead zone,
отключение/подключение, переназначение, блокирование ввода меню, stereo, volume,
mute и AI_LEN. Нет ROM и графического вывода.

`python3 tools/check_port_menu.py --sanitizer address` и `--sanitizer thread`
проверяют сохранение/перечитывание/повреждение JSON, резервные кнопки, обмен
привязок, захват/отмену/оси/отключение, удержание при фокусе/меню и общие снимки.
Настоящий закреплённый ImGui создаёт только CPU draw data, без SDL окна/GPU:
640×480, 1280×720, 3840×2160, 480×900 с автоматическим масштабом; проверяются границы окна,
восстановление шрифта/стиля, вкладка Controls, клавиатурная и gamepad навигация.
Ошибочное содержимое JSON в stderr — намеренный тест восстановления.

Windows RT64 frontend и Linux RT64 frontend компилируются. Linux можно собрать,
используя уже подготовленный graphics-replay build (сам replay не запускать):

```sh
cmake -S research/graphics-replay -B build-local/graphics-replay \
  -DDORA3_BUILD_LIVE_STARTUP=ON \
  -DDORA3_GENERATED_DIR="$PWD/build-local/startup-v61" \
  -DDORA3_AUDIO_GENERATED_DIR="$PWD/build-local/audio-rsp-v1"
cmake --build build-local/graphics-replay --target dora3_startup -j4
```

Успешные сборки/компоненты не подтверждают живое изображение меню, смену
разрешения GPU, реальный звук/геймпад или прохождение. Это проверяет пользователь
в test-021. Linux packaging и Android остаются следующими этапами.
