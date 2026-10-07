# Windows build без тестов после050

Основной репозиторий: /home/vova/projects/DoraRecomp3, main.
Текущий EXE055 собран изc8d6d32 с правкой6cd905f. Старые проверки не запускать.
Актуальная CPU generation startup-music-handoff-v1, RSP audio-rsp-v1 прежний.
Использовать готовую generation и Windows cache для следующих C++-only сборок.
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
2. Перед обновлением staging сверить его файлы со stage-manifest.json и
   сохранить отличающиеся исходники/метаданные архивом. Старые generation
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
6. Обновить документы и commit/push существующей main. Не запускать EXE.

Локальные материалы051: build-local/audio-test051-build/ (stage manifest,
архив исходников050 перед staging, Windows build log, package-before.json,
package-preservation.json). Это не CPU tests и не игровое подтверждение.
