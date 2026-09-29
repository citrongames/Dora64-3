# VI и отложенное освобождение ресурсов — test-018

## Исправленная гонка VI

Первичный источник: зафиксированный N64ModernRuntime, ultramodern/src/events.cpp.
SDK setters изменяли ViState под message_mutex; VI worker менял cur_state и
states[] под другим regs_mutex, а getters current/next framebuffer читали без
блокировки. Это гонка C++, независимо от корректности гостевого scheduler.

В tools/prepare_runtime_timing.py getters и dummy VI setup теперь используют
message_mutex, retrace block берёт message_mutex и regs_mutex через scoped_lock.
Reset уже брал оба таким же способом. Вложенного обратного порядка не добавлено.
Исходники зависимостей и Dora64 не изменены; адаптация создаётся при сборке.

tools/check_vi_sync.py включает настоящий events.cpp и извлекает точный retrace
block. Три потока по30000 итераций вызывают setters, retrace и getters; дополнительно
проверяются swap, black, repeat и VI mode. ThreadSanitizer воспроизвёл гонку
update_vi/get_cur_state на исходной версии (exit24). Исправленная версия прошла
с exit0 без сообщений TSAN. Рендерер и игра не запускаются. Предупреждение GCC
о неподдерживаемом atomic_thread_fence относится к инстанцированному коду очереди;
очередь в этом стенде не используется и её безопасность им не доказана.

## Карта ресурсов и границы доказанного

| Область | Проверенное поведение |
|---|---|
| 801AEC00 | 512 записей по20 байт: pointer, resource ID, size, ref16, pending decrement16, countdown8 |
| 80030AE8 | Повторное получение ресурса увеличивает refcount и сбрасывает countdown; новый ресурс выделяется и читается/распаковывается в отдельную область |
| 80030D00 | При нулевом refcount выставляет16 и сразу уменьшает countdown; по истечении вызывает80030C44 |
| 80030C44 | Освобождает ресурс при нулевом refcount, обнуляет запись и ставит ID=-1 |
| 800DD420 | 100 записей по8 байт: pointer и countdown |
| 80038B10 | Записывает pointer и16 в свободную запись; при полном массиве возвращается без регистрации |
| 80038B4C | Уменьшает countdown, при нуле вызывает free и обнуляет запись |

Стенд check_resource_retirement.py исполняет оригинальные generated-функции с
синтетической RDRAM и регистрирующим free endpoint. Оба пути освобождают ресурс
после16 ticks даже при outstanding tasks=2. При101 запросе второй реестр хранит100;
после16 ticks поступает100 free calls. ASan/LSan прошли, но потерянный запрос —
семантическое наблюдение в гостевом коде, не обнаруженная host-утечка санитайзера.
Достижимость этих состояний на реальном игровом пути пока не установлена.

В test-018 существующий entry trace80038B10 проверяет заполненность массива.
При переполнении пишет RESOURCE RETIRE TABLE FULL с pointer, outstanding и
счётчиком; сообщения только на степенях двойки. Фиксированный счётчик не создаёт
растущую очередь. Оригинальный алгоритм освобождения не изменён: диагностика
не является исправлением переполнения. Таймер не объявляется заменой GPU fence.
Наивное ожидание outstanding=0 может удерживать ресурсы постоянно при непрерывной
очереди. Для исправления возможной гонки нужны точное владение вложенными
DL/вершинами/текстурами и граница завершённого чтения конкретной опубликованной задачи.

## CPU framebuffer и raw RDP

Аудит прямых JAL всех4778 оригинальных функций,49 overlay-регионов:
osViGetCurrentFramebuffer800595F0 и osViGetNextFramebuffer80059630 вызываются
по одному разу из80026CEC (проверка планировщика). osDpSetNextBuffer800586F0 имеет
один прямой вызов80026EF0 из80026E24. Обычный producer8002FE20 использует flags63,
но это не доказывает недостижимость альтернативной raw-RDP ветви через другие
пути/косвенные вызовы. Её визуальная работа не подтверждена.

Чтения framebuffer pointer в scene init часто задают верхнюю границу heap;
их нельзя считать CPU-записями изображения.80043390 очищает область от801B3770
до текущего heap base. Известная переходная копия800271B0 защищена в test-017.
Полный набор других CPU writers и срок жизни вложенных ресурсов остаются открыты.

## Воспроизведение без запуска игры

```sh
python3 tools/check_vi_sync.py --baseline  # ожидается TSAN race, exit24
python3 tools/check_vi_sync.py             # исправленная версия, exit0
python3 tools/audit_resource_lifetime.py --rom "$ROM"
python3 tools/check_resource_retirement.py --generated build-local/startup-v61
```

Отчёты: research/vi-sync-check.json, resource-lifetime-audit.json,
resource-retirement-check.json, windows-build-check.json. Полные логи и
извлечённые оригинальные тела остаются в игнорируемом build-local.
Windows RT64 и Linux diagnostic собраны; EXE не запускался. CPU generation v61
и audio-rsp-v1 неизменны. Проверены32 зависимости и неизменность Dora64.
Расход GPU/VRAM и длительное прохождение требуют отдельных наблюдений.
