# Живой RT64 и подготовка стартовой сцены

## Проверено 2026-09-27

Исследовательский Linux frontend использует тот же закреплённый RT64, что и
replay. В WSLg/Vulkan на RTX2060 setup завершился успешно, первая настоящая
F3DEX2 task (DL 80120638, 0x30 байт) разобрана в живом запуске. Оригинальный
scheduler обработал SP и DP completion. Это пока команды настройки графики,
не видимый кадр игры. Свидетельство: research/live-startup-check.json.

163 оригинальные функции, 43 SDK mappings, 15 установленных indirect targets,
69 явных fail-stop границ. Неизвестные функции не получают фиктивный успех.
В раннем v24 выполнены две audio RSP tasks и получены512 стереокадров тишины
(research/history/startup-v24-live-initial.json). В итоговом бинарнике до
fail-stop успела завершиться одна audio RSP task; PCM callback ещё не достигнут. PCM измеряется и отбрасывается, аудиоустройства нет.
Контроллеры по-прежнему отключены.

SaveType настроен Eep16k: оригинальный startup требует probe==2. Прошли чтение
трёх слотов по512 байт, проверка подписи и подготовка пустых слотов в RAM.
Запись EEPROM, повторная загрузка непустого сохранения и ветви checksum пока
не проверены. Все данные запуска остаются в отдельном свежем run directory.

800433D8 выбрала scene ID0x22 (34). Jump-table entry33 по8007DC20 указывает
на80043534: последовательно вызываются800430EC и80043154. Реальные DMA:

| ROM | RAM | Размер | Проверка |
| --- | --- | --- | --- |
| 0x5E400 | 0x801B3770 | 0x51C0 | побайтно совпало |
| 0x635C0 | 0x801B8ED0 | 0xD490 | побайтно совпало |

Затем выполнение останавливается кодом77 перед800280DC. Эта функция устанавливает
процессы; следующий callback —801C0184, ROM6A874 во втором загруженном оверлее.
Его тело ещё не рекомпилировано/зарегистрировано. Оверлеи загружены, но их
исполняемый код ещё не запускался. Следующий этап — отдельные sections,
проверенные границы функций и регистрация при загрузке/замене оверлея.

## Память и завершение задачи

Закреплённый events.cpp вызывает sp_complete до send_dl. Его комментарий
относится к первой Doraemon: перенос completion после разбора не исправил
её nested-list corruption и задерживал игру. Для третьей игры это не доказательство
того же устройства буферов. Здесь используется ограниченный эксперимент:
SP completion после send_dl, затем штатный DP completion.

tools/prepare_runtime_timing.py проверяет точный SHA256 исходного events.cpp,
делает две однозначные замены и создаёт events-after-parse.cpp только внутри
build-local. CMake компилирует эту копию в режиме DORA3_LIVE_RT64. Default
диагностическая сборка использует неизменённый runtime. Все32 dependency pins
и исходники библиотек, включая рабочий Dora64, остаются прежними.

Это не универсальное исправление гонок и не подтверждение оригинального темпа.
Ожидание SP ограничивает одну точку повторного использования; основному CPU
оно не запрещает независимые записи, не доказывает время жизни nested lists,
матриц, света, textures и GPU framebuffer feedback. Снимки/барьеры для реальных
сцен предстоит определить по третьей игре. Патчи камеры/пулов первой игры
не перенесены. Первые state-only tasks недостаточны для такой проверки.

Live mode ограничен32 задачами либо15 секундами после setup. При fail-stop
до2 секунд даётся уже принятой графической задаче завершить разбор; игровой
вызов при этом не возвращает успех. Приватный capture с VI поддерживается
отдельным default diagnostic target; DORA3_CAPTURE_GRAPHICS с live отвергается.
Диагностическое аварийное завершение не проверяет shutdown и сохранение cache.

## Воспроизведение

Из корня проекта после bootstrap зависимостей/генератора. Значение ROM — свой
файл с проверенным хэшем. Для повторения выбирать свежие generation/run пути.
Нужны Linux, SDL2 development files и Vulkan; Windows/Android live ещё не собраны.

~~~bash
python3 tools/generate_startup.py --rom "$ROM" --output-dir build-local/startup-live-new
python3 tools/generate_audio_rsp.py --rom "$ROM" --output-dir build-local/audio-live-new
cmake -S research/runtime -B build-local/live-new -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DDORA3_LIVE_RT64=ON \
  -DDORA3_GENERATED_DIR="$PWD/build-local/startup-live-new" \
  -DDORA3_AUDIO_GENERATED_DIR="$PWD/build-local/audio-live-new"
cmake --build build-local/live-new --target dora3_startup -j6
python3 tools/run_startup.py --live --rom "$ROM" \
  --executable build-local/live-new/dora3_startup --run-dir build-local/run-live-new
~~~

Чтобы переиспользовать уже собранный RT64 replay target, можно вместо этого
конфигурировать research/graphics-replay с DORA3_BUILD_LIVE_STARTUP=ON и теми же
DORA3_*_GENERATED_DIR. Executable будет runtime-startup/dora3_startup внутри
этого build directory. Именно такой вариант проверен в build-local/graphics-replay.
Runner даёт setup до180 секунд; результат и журнал сохраняются локально.
Ожидаемый текущий результат —77, pending800280DC и две проверенные overlay DMA.

Старый diagnostic target с DORA3_LIVE_RT64=OFF отдельно пересобран с теми же
CPU sources и остановился на78 в graphics callback до audio RSP submission. Metadata:
research/history/startup-v24-diagnostic.json. Исторические v20/replay receipts
не заменяются: они относятся к другим бинарникам и снимку первой задачи.

Количество успевших выполниться аудиозадач до диагностической остановки
зависит от планирования потоков хоста. Проверка default mode требует ожидаемой
graphics/PCM границы78, а не обязательного PCM в каждом коротком запуске.
Первоначальное более строгое условие теста ошибочно требовало audio completion;
по журналу первая graphics task пришла раньше отправки audio. Условие уточнено
без обхода игровой функции или повторов ради нужного количества callbacks.
