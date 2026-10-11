# Android: перенос и общие Release-пакеты (2026-10-11)

Основа сравнения — Dora64 680d81feb4c57309ba80536300d73725ea9dffaa.
Его проект, библиотеки, ключи и пользовательские данные не изменялись.
Lock Dora64-3 сохранён. Проверка устройств из документации первого порта
не означает проверку Dora64-3 на этих устройствах.

## Интеграция

Android — SDL2 Activity + libDora64Three.so, собственные ID/иконка/хранилище,
системный выбор и проверка ROM, локальные переводы и индикатор компиляции
шейдеров. Окно принадлежит SDL/Android; desktop mode/monitor/relative mouse
capture не применяются. Меню имеет крупные кнопки, независимую прокрутку тела,
закреплённый footer, вкладку Touch. Desktop меню сохраняет прежний размер.

Touch state/layout/renderer перенесены из первого порта с адаптацией к input
и camera Dora64-3: DPI-база720, общий коэффициент для плотных экранов,
сохранение короткого касания до чтения гостем, владение пальцами, редактор
раскладки, отмена контактов при смене контекста/фокуса. Default stick y=.74,
A/B y=.79; существующий JSON не переписывается. P1 touch дополняет физическое
управление; геймпады P2–P4 не скрывают его.

Диалог определяется существующим ROM busy byte 0x8021710D только при overlay
0x70A50. Оригинальные setters/query/reset находятся в funcs_18 (801B91D4,
801B9200, 801B922C, 801B9238). Modern camera по-прежнему использует общую
eligibility всех платформ, Original/cutscene/minigame исключения сохранены.

Аудио останавливается после получения SDL lifecycle event, вне event watcher;
смена audio rate откладывается до resume, очереди очищаются. Native producer
на NDK27 использует std::thread + atomic stop и прежний wake signal, поскольку
stable libc++ этого NDK не предоставляет jthread. Shutdown уведомляет через
тот же mutex/revision и join; desktop сохраняет std::jthread. Логика дедлайнов
Original/Native не менялась. Android default и Modern = Manual60, Original
profile сохраняет Original FPS.

## Общие исправления из закреплённых библиотек

| Слой | Закреплённая версия и перенесённая политика |
| --- | --- |
| RT64 | 89cda7a5cc19c2ac7dcd0fb19cf70f34886a5c31: мобильные fixed texture bindings без descriptor indexing, Adreno texture/shader fallback, coverage MRT fallback без dual-source blend, persistent pipeline cache |
| Plume | f73ce68bd5cc4ea2dd182eb1150c9514658705b3: Vulkan flush/invalidate noncoherent memory, attachment dependencies и мобильные compatibility paths |
| N64ModernRuntime | 14bec16b314a24e19dab0c98232cff4587670ef3: Android runtime/thread support; Dora64-3 использует собственные копии адаптеров |

В закреплённом RT64 также присутствует Android StructuredBuffer/RWStorage
доступ к native framebuffer и packed8/16 форматам без oversized texel views,
а также очищение вновь выделяемых targets. Это исправления классов ошибок
Adreno512/660, Mali и Maleoon из донорской истории; фактический результат
здесь ещё проверяется владельцем. Диагностический CPU-affinity probe не включён.

Снимки памяти и указатели первой игры НЕ заменяют архитектуру Dora64-3:
сохраняются её resource banks, сигнал SP после parse, graphics RDRAM mutex
и взаимное исключение submit в Android. Все runtime-specific adapters
генерируются в build-directory, pinned checkout не редактируются.

## ROM и пакеты

Windows/Linux без аргументов сканируют каталог executable: любой регистр
расширения .z64, 16 МиБ и XXH3_64 = F3BFFF0C58827510. Неподходящие кандидаты
пропускаются; несколько подходящих выбираются в стабильном порядке имени.
ROM не переименовывается. Android document picker проверяет SHA256 при
импорте, runtime повторно сверяет размер/XXH3. ZIP/byte-swapped ROM не принимаются.

Все три пакета собираются в CMake Release, без debug-info, диагностики и
профилировщика. Android optimized имеет non-debuggable manifest и тестовую
подпись для локальной установки. Архивы не содержат ROM, пользовательских
настроек/сейвов, PDB, исходников и compile logs. Сборочные receipts остаются
в проекте/локальной рабочей папке. Linux требует совместимый системный ABI
и Vulkan-драйвер; переносимость не проверяется запуском в рамках этой задачи.

Компиляция и статическая проверка упаковки не подтверждают игровой результат.
Коммит/push ждут проверки и отдельной команды владельца. Релиз на GitHub
не создаётся.

## Результат сборки

Windows test116 Release, Linux x86_64 Release и Android ARM64 Release успешно
скомпилированы. Архивы в D:\Games\DoraRecomp3\release-test-116, SHA256SUMS рядом.
Windows не содержит PDB/CodeView; оставлена служебная POGO-запись оптимизации.
Android manifest non-debuggable, APK-подпись проверена; в обеих native SO нет
.debug_/symtab. Linux stripped ELF без debug/symtab, требует glibc2.43+
(среда Ubuntu26.04) и системные SDL2/GTK3/libstdc++/Vulkan.
Старые512 файлов testbuild сохранены; добавлены test116 EXE и Android-test001 APK.
Не запускались тесты, CPU checks, игра, renderer, GPU или эмулятор.

2026-10-11: пользователь разрешил коммит/push блока через test118 в main.
Покрытие конкретных устройств не уточнялось. На Git-шаге новых сборок и
запусков нет; предыдущие пометки ожидания относятся к этапам разработки.
