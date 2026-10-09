# Чистый замер после074 —075

Прямая инструкция пользователя: следующий Windows build сделать без проверок
и логов, чтобы измерить скорость без диагностической нагрузки. Это отдельный
контрольный замер; он не выдаётся за исправление всех оставшихся расходовRT64.
В074 подтверждены60FPS и принятиеManual360. Универсального100FPS cap нет.
Профиль указывает на producer графики:5–8мс CPU/кадр в тяжёлых сценах, но
обёртка не отделяет interpreter/fullSync/подготовку/очереди/собственныйlog.
Логи и измерители тоже добавляют расходы; их доля до этого не измерена отдельно.

## Что выключено в075

Windows builder по умолчанию DORA3_DIAGNOSTICS=OFF; исследовательская сборка
доступна явно через -Diagnostics. Новая генерация startup-clean-after074-v1
использует --no-diagnostics без --profile. Из 4778 исходных функций убраны
общие trace-only entry hooks; сохранены12 ROM-qualified функциональных adapters.
Их роли и инструкции: research/runtime-trace-adapters.json. Ни адреса, ни
назначения не перенесены из другой игры. Нет14 C profiling wrappers.
Все прочие hook texts и instruction patches совпадают с074, статическое
сравнение: research/test075-clean-source-review.json. СгенерированныйC не
редактировался вручную.

В C++ profiler безDORA3_PROFILE становится inline no-op: нет timers, atomic
counters, sampling, thread CPU queries илиJSON report. В очередяхRT64 тоже нет
наших scopes/counters. Собственныеprintf/fputs выключены на этапе компиляции,
включая вычисление форматирующих аргументов. Настоящие submit/completion counters
сохранены отдельно от логов, чтобы drain/очередь работали как прежде.

Нет создания logs/<session>, runtime/stdout/performance/controller/resource
журналов, снимков и периодических memory probes. stdout/stderr библиотек в
Windows направлены вNUL. Побайтовые сверки послеDMA и каждогоaudio RSP task,
per-function overlay entry checks и диагностический учёт первого входа выключены.
Startup ROM identification выполняется один раз; actual overlay lifetime/load,
Native ownership, retirement, игровые условия и защита протоколов очередей
остаются. Это часть исполнения игры, не экспериментальные проверки.

Original/Native/Manual, AI, contacts, время/границы событий, дальность и лимиты
не упрощались. FPS overlay продолжает считать реальные output frames. Настройки
не переключать при упаковке: для сравнения пользователь выбираетManual360/5x
в том же месте. После075 автоматического performance.jsonl нет; результатFPS
сообщает пользователь. Старый074 и его диагностика остаются рядом.

Только необходимая генерация и Windows компиляция/упаковка; игра, tests,
CPU checks, renderer/GPU агентом не запускались. До команды пользователя Git нет.

## Сборка завершена

Windows MSVC configure/compile/link: exit 0, 57.688 секунд, инкрементальный кэш сохранён.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-075.exe.
SHA-256: `4a53b62935f1c3e6d5e7cb882f05bc86f802cf2f395c69537f03f29611f36534`.
Все 237 прежних файлов пакета сохранены.
Игра, renderer/GPU, тесты и CPU checks не запускались.
FPS измеряет пользователь; автоматические журналы и профилировщик выключены.
Commit/push не выполнялись.
