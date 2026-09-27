# Воспроизведение текущего этапа

Нужны Python 3.10+, Git, CMake >=3.20, C/C++20 toolchain; GCC/Clang/MSVC.
Основное рабочее окружение — WSL Ubuntu. Команды из корня отдельного проекта.
Они не собирают пока игровой порт. Папки generation и файлы отчётов должны
быть **новыми**: повторный запуск выбирает другой suffix, существующие данные
не затираются. Ничего не скачивает ROM, не извлекает его в tracked directories.

```bash
python3 tools/bootstrap_n64recomp.py --jobs 6
python3 tools/bootstrap_dependencies.py
# На текущей машине вместо загрузки объектов из сети:
# python3 tools/bootstrap_dependencies.py --local-donor /home/vova/projects/Dora64

ROM='/mnt/d/Games/DoraRecomp3/Doraemon 3 - Nobita no Machi SOS! (J) [!].z64'
python3 tools/inspect_rom.py --rom "$ROM" --output build-local/rom-new.json
python3 tools/analyze_layout.py --rom "$ROM" --output build-local/layout-new.json

python3 tools/generate_boot.py --rom "$ROM" --output-dir build-local/boot-new
cmake -S . -B build-local/linux-new -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DDORA3_GENERATED_DIR="$PWD/build-local/boot-new"
cmake --build build-local/linux-new
./build-local/linux-new/dora3_boot_probe

python3 tools/generate_boot.py --kind decoder --rom "$ROM" --output-dir build-local/decoder-new
cmake -S . -B build-local/decoder-linux-new -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DDORA3_PROBE_KIND=decoder -DDORA3_GENERATED_DIR="$PWD/build-local/decoder-new"
cmake --build build-local/decoder-linux-new
python3 tools/verify_decoder.py --rom "$ROM" \
  --probe build-local/decoder-linux-new/dora3_decoder_probe \
  --layout build-local/layout-new.json --output build-local/decoder-check-new.json
```

Для опционального XXH3-анализа микрокодов требуется host shared library xxhash:

```bash
python3 tools/identify_microcode.py --rom "$ROM" --output build-local/ucodes-new.json
```

Генератор сверяет ROM, pin/patch/binary receipt и пишет хэши metadata/scripts/
generated files. CMake отвергает изменённые inputs или generated output.
Смена native harness не требует новой генерации игрового C.
Git игнорирует build-* включая build-deps и локально сгенерированный код.

## Диагностический настоящий runtime — Linux

После bootstrap инструментов и зависимостей:

```bash
python3 tools/generate_startup.py --rom "$ROM" --output-dir build-local/startup-new
python3 tools/generate_audio_rsp.py --rom "$ROM" --output-dir build-local/audio-new
cmake -S research/runtime -B build-local/runtime-new -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DDORA3_GENERATED_DIR="$PWD/build-local/startup-new" \
  -DDORA3_AUDIO_GENERATED_DIR="$PWD/build-local/audio-new"
cmake --build build-local/runtime-new -j6
python3 tools/run_startup.py --rom "$ROM" \
  --executable build-local/runtime-new/dora3_startup \
  --run-dir build-local/run-startup-new
```

Ожидается DMA VERIFIED для обоих банков, затем диагностическая граница.
В v20: RSP TASK type=2 bytes=9A0, AUDIO RSP COMPLETED reason=1,
оригинальный SP completion и graphics/PCM boundary78. Первый PCM:
samples=1024, frames=512, nonzero=0. Порядок потоков может оборвать журнал
до PCM callback; отсутствие строки нельзя выдавать за его выполнение.
Отсутствие RSP completion в конкретном журнале нельзя выдавать за выполнение.
Runner сохраняет observations: задачи, штатные RSP completion, DMA и unknown
calls. Журналы остаются в build-local.

Runner возвращает 0 для предусмотренных границ 77/78: это не запуск игры.
Коды 79/80/81/82/83/124 означают runtime timeout, runtime error, DMA mismatch,
audio RSP failure, ошибку записи снимка и runner timeout; runner вернёт 1.
При остановке даётся до 250 мс уже отправленной аудиозадаче, без успешного
возврата непроверенной функции. Новая state-папка — на каждый запуск,
сохранения отключены. Windows/Android для frontend ещё не проверены.

Оба генератора проверяют ROM, pin/binary receipts и пишут хэши входов/выходов.
Аудиогенератор проверяет text hash и 16 целей переходов из ROM; чужой generated
aspMain не используется. CMake проверяет оба комплекта и связывает audio RSP
с frontend. Отдельная компиляция RSP object больше не требуется.

## RT64 replay — Linux x86-64

Команды и ограничения: [graphics-replay.md](graphics-replay.md).
Используется приватный снимок собственной первой задачи. Не добавлять его
в Git и не выдавать replay за работающую игру.

## Windows

На native checkout со своим Python/Git/VS2022 использовать `py -3` вместо
`python3`, Windows ROM path и отдельную нативную сборку инструментов.
После генерации:

```powershell
cmake -S . -B build-local/windows -G 'Visual Studio 17 2022' -A x64
cmake --build build-local/windows --config Release
& .\build-local\windows\Release\dora3_boot_probe.exe
```

На первом этапе проверены компиляция и запуск того же generated C через MSVC
на Windows. Полный native Windows bootstrap проекта заново не выполнялся;
проверенный исходник для этой пробы был сгенерирован WSL-инструментом.
Не смешивать build directories и tool receipts Windows/WSL.

## Android — пока только компиляция probe

```bash
cmake -S . -B build-local/android-new -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/home/vova/Android/Sdk/ndk/27.0.12077973/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 \
  -DCMAKE_BUILD_TYPE=Release -DDORA3_GENERATED_DIR="$PWD/build-local/boot-new"
cmake --build build-local/android-new
```

NDK путь локальный, на другой машине указать свой. Это executable для проверки
toolchain, не игровой APK. Package/SDL activity/ROM picker/подпись появятся после
минимального игрового цикла. Пользователь сам устанавливает и тестирует APK.
