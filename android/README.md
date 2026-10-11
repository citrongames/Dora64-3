# Android

Игровой порт Dora64-3, ARM64, Android 7.0/API24 и Vulkan1.0. Первая сборка
предназначена для локальной проверки владельцем; совместимость устройств
ещё не подтверждена. Игра/тесты/эмулятор агентом не запускаются.

## Сборка

Нужны JDK17, Android SDK36, build-tools35, NDK27.0.12077973, CMake3.22.1,
Python3 и host file_to_c, скомпилированный из закреплённого RT64. Зависимости
и чистая CPU/RSP генерация — по ../docs/building.md. Из корня проекта:

```sh
export JAVA_HOME=/path/to/jdk17
export ANDROID_HOME=/path/to/Android/Sdk
export DORA3_HOST_PYTHON=/usr/bin/python3
export DORA3_HOST_FILE_TO_C=/path/to/host/file_to_c
./android/gradlew -p android --max-workers=4 :app:assembleOptimized \
  -Pdora3CpuDir=../../build-local/startup-clean \
  -Pdora3AudioDir=../../build-local/audio-rsp
```

`optimized` использует CMake **Release**, debuggable=false, jniDebuggable=false,
явный -g0 (NDK по умолчанию добавляет -g), выключенные DORA3_DIAGNOSTICS и DORA3_GRAPHICS_PROFILE. Native-библиотеки APK
очищаются Android packaging от таблиц символов. Для локальной установки APK
подписан стандартным тестовым ключом, что НЕ включает debug-код/флаг приложения.
`release` использует тот же код; отдельная production-подпись задаётся через
DORA3_SIGNING_PROPERTIES (storeFile/storePassword/keyAlias/keyPassword).
Ключ первого Dora64 не читается и не используется. Без заданного ключа этот
вариант остаётся неподписанным. Публикация в Store/GitHub не выполняется.

Результат: app/build/outputs/apk/optimized/app-optimized.apk.
Application ID: com.n64recomp.dora64three. Соседний Dora64 имеет другие данные.

## Первый запуск

Выбери собственный японский N3DJ rev0 `.z64` любого имени. Импорт проверяет
16 МиБ и SHA256 410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
ROM копируется во внутренние данные приложения; оригинал не меняется.
Настройки/сейвы: Android/data/com.n64recomp.dora64three/files, локализация —
во внутренних assets. Общий доступ к хранилищу не требуется.

Первый запуск и Modern: Manual60 FPS. Original сохраняет исходную частоту.
Touch: шестерёнка открывает меню, редактор позволяет двигать/масштабировать
кнопки; раскладка сохраняется в port_touch_settings.json. По умолчанию стик
на y=0.74, A/B на y=0.79. Ранее сохранённая раскладка не перезаписывается.
Свободная зона управляет современной камерой; вне её разрешённых сцен
сохраняется оригинальное управление. Активность первого геймпада скрывает
тач, остальных игроков — нет. Четыре физических геймпада поддерживаются.

Архитектура и перенесённые исправления: ../docs/android-port.md.
