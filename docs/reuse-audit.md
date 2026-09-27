# Аудит переиспользования Dora64

Dora64 local HEAD и remote HEAD на момент проверки:
`680d81feb4c57309ba80536300d73725ea9dffaa`. Рабочее дерево чистое до/после работы.
Чтение обязательных шести документов зафиксировано SHA-256 в dependencies.lock.json.
Исторические разрешения на push/релизы/действия из документов не исполнялись.
Релиз v1.0.4 — историческая запись: desktop 1.0.5, Android 1.0.3 по сведениям
пользователя и итоговой документации. Ранние абзацы README о 1.0.2 устарели.

## Зафиксированные зависимости

| Компонент | Commit |
|---|---|
| N64ModernRuntime | 14bec16b314a24e19dab0c98232cff4587670ef3 |
| N64Recomp внутри runtime | 7c01d5c01307430bf7038e527bb16507e5f3448d |
| RT64 | 89cda7a5cc19c2ac7dcd0fb19cf70f34886a5c31 |
| plume | f73ce68bd5cc4ea2dd182eb1150c9514658705b3 |
| SDL2 (Android) | 5d249570393f7a37e037abf22cd6012a4cc56a71 |
| Генератор N64Recomp/RSPRecomp | ffb39cdad1da5de07eaaa48bd1db4a89a7986771 + zero-register.patch |

Полная рекурсивная фиксация всех зависимостей — dependencies.lock.json.
Независимые объекты Git и checkout лежат в ignored `build-deps/`; они не зависят
от сохранности каталога Dora64. Remote нового верхнего репозитория не задан.

Генератор взят на той же upstream-базе, но переносится только общее исправление
записи в MIPS zero с сохранением вычисления выражения. Семь принудительно
генерируемых SDK-функций, классификация SI/PFS и game.patch первой игры не перенесены.
Bootstrap переиспользован выборочно; лицензия N64Recomp сохранена.

## Что можно использовать как основу

| Механизм | Решение и граница |
|---|---|
| Vulkan non-coherent map/unmap, invalidate/flush, transfer→host barrier | Сохранить в plume; общая корректность API |
| Attachment LOAD/STORE dependencies | Сохранить; проверять на настоящих задачах третьей игры |
| Инициализация новых/изменивших размер color targets | Сохранить; не очищать каждый существующий кадр |
| Отсутствие dual-source blend | Сохранить capability-based separate coverage и корректные seed/merge |
| Малые descriptor limits/nonuniform | Сохранить local RGBA/TMEM slots, неизменность дескрипторов до завершения GPU |
| CPU framebuffer formats/texel limits | Сохранить Android packed storage buffers и форматно согласованные intermediates |
| Vulkan pipeline cache | Сохранить версионирование, driver/device key, атомарную запись; дать новому приложению своё хранилище |
| Adreno SampleGrad fallback | Сохранить bounded retry и специализацию; не выбирать по имени телефона |
| SDL ввод/звук/window/lifecycle | Архитектура применима; callbacks, app IDs, пути сохранений и тайминг переинтегрировать |
| Touch ownership/editor/JSON | Переносима общая логика; расположение HUD, диалоги, R-button и современная камера требуют новых сигналов |
| Port menu/профили/переназначение | Переносить после базового цикла, только реализованные настройки |
| Генерация из ROM + хэши + fresh output | Переиспользуется принцип; старые syms, registration и fingerprint не применимы |

## Что не является готовым общим исправлением

Защита кадров распределена между runtime mutex, API snapshot в RT64 и игровым
производителем snapshot в `src/doraemon_lighting.cpp`/patches. В render context
первой игры вручную задаются диапазоны display list, frame assets, sky, matrix/light
callbacks. Копирование только RT64 не гарантирует безопасный срок жизни памяти;
копирование старых адресов гарантировать корректность тем более не может.

`src/rt64_render_context.cpp` также содержит обработчики матриц/света/неба,
пулы и локализацию. Это смешанный слой, не готовый generic frontend.
`main.cpp` фиксирует 8-МиБ ROM, XXH3, вход 0x80000400, aspMain и регистрацию.
`system_overlay.cpp` напрямую вызывает игровые функции камеры/читов/языка.

Не переносить без отдельного исследования: 123 функции game.patch, 1205 символов,
1138 регистраций, pool expansion, culling/LOD thresholds, projection/HUD,
интерполяционные matrix IDs, camera ownership/collision, состояние диалога
0x800E6B20, glyph/font/table layout, тексты/текстуры, HP/lives/race cheats,
autosave/reset hooks и адреса framebuffer.

Даже зафиксированные форки не полностью универсальны: runtime содержит
game-specific диагностические адреса в PI trace и специфические callbacks.
Перед игровым подключением отделить эти места. Development probes первой игры
выключены в релизе через DORA64_ANDROID_DIAGNOSTICS; для новой игры не включать
старые affinity/timing probes как штатную политику.

## Android-выводы из обязательной документации

Итоговая общая проверка относится к 1.0.3-rc1 на восьми устройствах и двум уровням.
Полное прохождение на Lenovo 2025 относится к первой игре. Финальная 1.0.3
сохраняет исправления RC1, но меняет Android default FPS на Original.
Эти результаты не являются проверкой Doraemon 3 или обещанием тех же FPS.
Сначала новый package ID/save/cache namespace, затем отдельная матрица тестов
пользователя. Никакие APK в рамках первого этапа не устанавливались.
