# Сборка из исходников

Актуальный игровой target — `dora3_startup` в `research/runtime`.
Корневой CMake собирает ранние probe-инструменты. Команды ниже предназначены
для компиляции; они не запускают игру, тесты, CPU checks или GPU.
Новый чистый checkout в рамках уборки Git не собирался; команды сверены
с текущими генераторами и сборочными файлами.

## Зависимости и генерация

Нужны Python3.10+, Git, CMake3.20+, C/C++20 toolchain. Windows: Visual Studio2022
с C++ x64 tools и CMake/Ninja. Для переэкспорта графики нужен Pillow; готовые
авторские assets уже включены в Git. Используй инструменты своей ОС: CMake cache
и receipts инструментов WSL и Windows не взаимозаменяемы.

Из корня проекта, на Windows заменив `python3` своим Python:

```sh
python3 tools/bootstrap_dependencies.py
python3 tools/bootstrap_n64recomp.py --jobs 6
python3 tools/generate_startup.py --rom /path/to/original.z64 --output-dir build-local/startup-clean --no-diagnostics
python3 tools/generate_audio_rsp.py --rom /path/to/original.z64 --output-dir build-local/audio-rsp
```

Пути ROM заменить собственными. Обе generation-папки должны быть новыми;
после изменения правил генерации выбери новый suffix. Не редактируй полученный
C/RSP вручную. Компиляция проверяет хэши inputs и generated output.
Для обычного изменения native src/ повторная генерация не нужна.

## Windows

Из корня Windows checkout после генерации:

```powershell
.\tools\build_windows.ps1 -Python 'C:\path\to\python.exe' `
    -CpuDirectory 'build-local/startup-clean' `
    -AudioDirectory 'build-local/audio-rsp' -Jobs 6
```

Исполняемый файл: `build-local/windows/Dora64-3.exe`. Скрипт использует
инкрементальную сборку, диагностика и GraphicsProfile выключены по умолчанию.
Параметры generation-папок указаны явно: старые значения по умолчанию относятся
к исторической локальной сборке. `-Fresh` нужен только при необходимости.
Приложение скриптом не запускается.

`tools/package_windows.ps1` обслуживает существующий test-workflow: он требует
согласованные stage-manifest.json и research/windows-build-check.json от той же
сборки. Это не универсальный упаковщик для чистого clone. Не подменять receipts
старой сборки и не выдавать тестовый каталог за публичный релиз. Существующий
Windows staging и его кэш при уборке Git сохранены.

## Linux

После собственной Linux-генерации, из корня:

```sh
cmake -S research/runtime -B build-local/linux -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDORA3_LIVE_RT64=ON \
  -DDORA3_DIAGNOSTICS=OFF -DDORA3_GRAPHICS_PROFILE=OFF \
  -DDORA3_GENERATED_DIR="$PWD/build-local/startup-clean" \
  -DDORA3_AUDIO_GENERATED_DIR="$PWD/build-local/audio-rsp"
cmake --build build-local/linux --target dora3_startup -j6
```

Нужны системные development-пакеты, которые требуют закреплённые SDL2/RT64
для выбранного Linux окружения. Этот путь не означает проверку115 на Linux.
Готовой Android-игровой сборки пока нет.

История ранних probe/диагностических команд: [reproduction.md](reproduction.md).
Они не являются текущей обязательной последовательностью сборки.
