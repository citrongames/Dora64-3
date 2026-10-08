## Следующая сборка: Reset game после059

Reset реализован и скомпилирован по translation units без Windows EXE.
Следующая CPU generation: build-local/startup-menu-reset-v1 (audio-rsp-v1
прежняя); использовать актуальный tools/prepare_runtime_timing.py.
В Windows staging добавить src/game_reset.cpp и src/game_reset.hpp, новый
CMake source и обновлённые src/tools. Нужна новая явная команда на билд;
номер определить по фактически существующим EXE. Подробности: menu-reset.md.

## Действующее правило Git: только после пользовательской проверки

Прямая инструкция пользователя: «коммит и пуш делай только после моей проверки
а не сразу с билдом или в конце фикса». Это отменяет прежнее автоматическое
сохранение законченного fix/build через commit/push до игровой проверки.
Исправления, документацию и сборки до подтверждения пользователя оставлять
незакоммиченными в существующей main; сохранять их и не делать reset/clean.
Готовый fix, успешная компиляция или команда «билд» сами по себе НЕ разрешают
commit/push. Только после подтверждения результата пользователем сохранять
соответствующий проверенный этап коммитом и отправлять origin/main.
Правила без тестов и без локального game/renderer/GPU запуска остаются в силе.
Не отменять ранее сохранённые коммиты без отдельной просьбы пользователя.

## Рабочая Windows-копия и лишние архивы (обсуждение059)

Основной Git остаётся в WSL; существующая MSVC/CMake сборка использует
Windows staging с Windows-путями. В него передаются текущие незакоммиченные
правки без commit/push. Это подготовка рабочего каталога компилятора.
После вопроса пользователя о переносе исходников договорённость в ответе:
далее не создавать полный ZIP архив неизменённых/уже сохранённых Git исходников.
Сохранять лишь отличающиеся незакоммиченные/чужие данные при наличии таких
расхождений, старые generation directories, receipts и прежние EXE/PDB.
Существующие исторические архивы не удалять. Перед059 полный архив уже был
создан; новых полных архивов для этой сборки больше не делалось.

# Windows build без тестов после050

Основной репозиторий: /home/vova/projects/DoraRecomp3, main.
Последний EXE059 собран из незакоммиченного snapshot поверх e97b7e6,
с общей alpha-фильтрацией спрайтов мира. CPU generation
startup-sprite-edges-after058-v1, RSP audio-rsp-v1 прежний. Полный Windows
compile/link/package exit0;189 прежних файлов вне logs сохранены.
Обновлённые RT64 sampler и CMake копируются tools/prepare_rt64_sprite_edges.py
в build-local/windows/rt64-sprite-edges; dependency checkout/pins прежние.
При staging включать новые незакоммиченные files через git ls-files --cached
--others --exclude-standard: shader helper/generator/report должны попасть
в manifest и source snapshot. Пользователь подтвердил059 и прямо разрешил
commit/push этого этапа. Следующие исправления сохранять после их проверки.
Последний Windows cache сохранён; PDB/EXE058 и staging архивированы.
Следующий EXE после нового «нет» либо прямой команды собрать, без тестов.
Ниже сохранены прежние указания после058; для будущего переиспользования
предпочитать актуальную generation after058-v1 при совпадении её inputs.

Последний EXE058 собран из96935fc: terminal poses моделей в Source/Native.
CPU generation startup-model-terminal-after057-v2, RSP audio-rsp-v1 прежний.
Для следующих C++-only сборок переиспользовать эту generation и Windows cache,
только если её входы совпадают. Старые startup-music-handoff-v1 и draft model
terminal v1 не содержат окончательную policy table/325 hooks; их receipts не
переиспользовать.058 собран по прямой команде; следующий build после нового
«нет» либо прямой команды собрать. Старые проверки не запускать.
Windows cache/manifest обновлены, PDB/EXE057 и исходники перед058 сохранены.
Для внешнего запуска package script использовать bundled PowerShell Core
из C:/Users/black/.cache/codex-runtimes/codex-primary-runtime/dependencies/native/powershell/pwsh.exe:
в legacy powershell.exe с унаследованным module path недоступен Get-FileHash.
Build logs захватывать на уровне OS subprocess, без PS stderr-to-error pipeline.
src/audio_music_handoff.cpp включён в research/runtime/CMakeLists.txt.
В новой generation только helper8023D730 получает scoped MSVC optimize off/on.

Готовый Windows staging и cache находятся здесь (это вспомогательная копия,
не основной Git и не справочное зеркало sources):
`C:/Users/black/Documents/Codex/2026-09-27/doraemon-nintendo-64-rom-windows-linux/work/windows-src`.

Python Windows:
`C:/Users/black/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`.
Visual Studio: `C:/Program Files/Microsoft Visual Studio/2022/Community`.
CRT051: `VC/Redist/MSVC/14.44.35112/x64/Microsoft.VC143.CRT` внутри Visual Studio.

1. Сверить main/status/последний EXE. Если изменился только C++ и хэши
   inputs существующих generation receipts совпадают, переиспользовать generation
   и готовый Windows cache (как в052); полная генерация не нужна. Иначе создать
   новую generation в WSL через
   tools/generate_startup.py --rom с ROM из testbuild и новым --output-dir
   под build-local.051 использует startup-audio-test051-v1 и audio-rsp-v1.
   Встроенные ROM/generation integrity guards — часть генерации; standalone
   check/test/prebuild tools не запускать.
2. Перед обновлением staging сверить его файлы со stage-manifest.json.
   Полный архив совпадающих Git исходников не создавать. Если есть чужие или
   незакоммиченные отличия, сохранить только их до обновления. Старые generation
   directories сохранять. Перенести необходимые исходники и новую generation;
   не менять build-deps pins. Записать новый manifest с source commit и хэшами.
   Compile receipt research/windows-build-check.json НЕ включать в собственные
   source_sha256 или immutable stage inputs: он создаётся после компиляции.
3. В Windows запустить tools/build_windows.ps1 из staging с -Python, новым
   -CpuDirectory и -Jobs6. Скрипт только configure/compile, без запуска игры.
   Лог051: staging/build-local/windows-test051-build.log.
4. Только после exit0 записать новый research/windows-build-check.json:
   реальный EXE SHA256, исходники, generation, completed build log hash,
   tests_executed=false, completed_cpu_checks=0. Прежний receipt сохранить
   в research/history; старые PASS не добавлять в новую валидацию.
5. tools/package_windows.ps1 -Destination D:/Games/DoraRecomp3/testbuild
   -CrtDirectory с существующим CRT -BuildId со следующим свободным номером.
   Скрипт сохраняет общие DLL, запрещает перезапись старого EXE и не копирует ROM.
   Сверить хэши прежних файлов; logs не являются immutable input.
6. Обновить документы; до игровой проверки пользователя оставлять fix/build
   незакоммиченными в main. После подтверждения результата — commit/push
   соответствующего проверенного этапа. Команда «билд» не разрешает commit/push.
   Не запускать EXE локально.

Локальные материалы051: build-local/audio-test051-build/ (stage manifest,
архив исходников050 перед staging, Windows build log, package-before.json,
package-preservation.json). Это не CPU tests и не игровое подтверждение.
