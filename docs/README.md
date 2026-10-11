# Документация Dora64-3

## Текущая работа

- [Состояние проекта и возможности](../README.md)
- [Сборка из исходников](building.md), [Android APK](../android/README.md)
- [Android: интеграция, touch и исправления](android-port.md)
- [Последняя Windows-сборка](windows-test-build.md)
- [Журнал текущей работы и приёмки](work-in-progress.md)
- [Правила Git](repository-policy.md), [изоляция библиотек](dependency-policy.md)

## Возможности и архитектура

- [Меню и профили](port-menu-controls.md), [четыре игрока](four-player-input.md)
- [Modern Camera](modern-camera.md), [Widescreen](widescreen.md)
- [Reset game](menu-reset.md), [автосохранение](autosave.md), [читы](cheats.md)
- [Переводы: реализация](localization.md), [формат языковых пакетов](../assets/localization/README.md)
- [Native FPS](native-game-loop.md), [модельные анимации](model-animation.md)
- [Жизненный цикл ресурсов](vi-resource-lifetime.md)

Тематические документы содержат историю по сборкам; более поздняя датированная
приёмка заменяет раннюю пометку «ожидается проверка». Старый PASS сохраняет
исходный контекст и не доказывает проверку последующих изменений.

## История

- [Исходное исследование](initial-investigation.md), [журнал исследования](investigation-log.md)
- [Ранние команды воспроизведения](reproduction.md)
- [README исследовательского этапа](history/readme-research-stage.md)
- [Архив указаний через test115](history/agent-instructions-through-test115.md)

Архив не является текущим заданием. Правила работы — в [AGENTS.md](../AGENTS.md).
Не удалять research/ и исторические fixtures ради уборки: среди них есть входы
генераторов и CMake игрового target. Все старые документы оставлены на прежних
путях, чтобы сохранить ссылки и происхождение решений.
