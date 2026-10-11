# Происхождение

Основа аудита: citrongames/Dora64, commit 680d81feb4c57309ba80536300d73725ea9dffaa.
Выборочно переиспользован `tools/bootstrap_n64recomp.py`; из патча генератора
выделено общее исправление регистра zero. Патчи самой первой игры не перенесены.

N64Recomp: https://github.com/N64Recomp/N64Recomp — MIT; notice сохранён
в tools/n64recomp/LICENSE.N64Recomp. Зависимости сохраняют свои собственные
лицензии/авторство в независимых checkout, перечислены в dependencies.lock.json.

Разработка порта, инструментов, переводов и документации выполнена citrongames
при существенном участии ChatGPT и OpenAI Codex. Направление проекта и игровую
проверку обеспечивает владелец; это не атрибуция авторства сторонних библиотек. Генерированный игровой код, оригинальный ROM
и распакованные игровые ресурсы не являются распространяемыми файлами проекта.
Лицензия будущего публичного верхнего проекта пока не выбрана; публикация не разрешена.

Авторские шрифты и локализованные Doraemon wordmarks переиспользованы из Dora64
по указанию владельца; исходный проект не изменялся. Происхождение оформления:
resources/artwork/README.md, resources/artwork/localization/README.md.
SDL2 и вложенные библиотеки перечислены в dependencies.lock.json; их лицензии
остаются в собственных checkout. Общая лицензия порта не заменяет и не отменяет
лицензии компонентов. Для будущего публичного бинарного выпуска требуется
отдельная комплектация notices и лицензий всех распространяемых компонентов.

Android Activity/Gradle infrastructure, mobile menu and touch state/layout/rendering
adapted from citrongames/Dora64 at the commit above. Donor files and dependency
checkouts were read-only; its game-specific address adapters were not copied.
