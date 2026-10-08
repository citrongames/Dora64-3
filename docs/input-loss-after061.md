## Причина установлена в следующем062 запуске; fix собран в063

Первый Start принимался до pause(7) инициализатора, после чего поток пропуска
опять останавливался и requested flag блокировал повторные нажатия. Общий
фикс для30 сценариев включён в063; пользовательский результат063 ещё ожидается.
См. [cutscene-early-start-after062.md](cutscene-early-start-after062.md).
Ниже — исторический первоначальный анализ061.

## Test062 готов: диагностика игровых обработчиков ввода (2026-10-08)

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-062.exe.
SHA256: 6b47f139f8e0f3c95b44d270c96ff94bb29ba534826236da82b7856719522a4b.
По прямой команде пользователя062 собран/упакован из dirty main09ffb36;
MSVC RelWithDebInfo configure/compile/link exit0. CPU startup-menu-reset-v1
и audio-rsp-v1 переиспользованы по совпавшим входам. Скопированы только
11 изменённых staging files; полного source ZIP нет, PDB/EXE061
и receipt061 сохранены. Все198 прежних файлов вне logs сохранили
размер/mtime; общие DLL проверены упаковщиком по хэшам. ROM/saves/settings
не менялись. Тесты и игра/emulator/renderer/GPU не запускались, commit/push нет.

controller-input.jsonl schema2 теперь видит Source dispatch/completion,
игровой/script/menu consumer, выбранные/latched buttons, counters/processes
и флаг/records пропуска заставки. Это диагностика ещё не установленной
причины отказа061; сам input loss не заявлен исправленным. Разбор:
[input-loss-after061.md](input-loss-after061.md). Пользователь проверяет062.
Новое правило пользователя закреплено в AGENTS.md: после завершённых
изменений кода сразу делать билд; Git только после пользовательской проверки.

# Игровой ввод после последнего reset в061

2026-10-08. Пользователь поймал отказ на третьем reset в одном процессе.
Main09ffb36, прежние dirty060/061 изменения сохранены. Нового061 дампа нет;
последний060 дамп относится к ранее установленному отдельному SDL shutdown bug.
Процесс061 к моменту чтения журналов уже закрыт; live RAM недоступна.

## Что подтверждает пользовательский запуск

logs/1791431816261/runtime.log — Windows test-061. Reset на строках5265,
10495,11867; четыре внутренних сессии. controller-input.jsonl содержит461
запись; последние polls по сессиям537/523/214/1027. В каждом after-read
port0 connected, query type5/error0, pad error0; host и raw guest buttons
совпадают во всех записях. Последнее вновь сформированное Start-нажатие
в session4 видно на poll859: raw4096, held4096, pressed4096; menu0,
focused1, ui_blocked0, host_enabled1. Кнопки и оси проходят после reset;
повторная правка SI presence/quarantine057 этими данными не обоснована.

Первый after-read observer находится на entry30380 после настоящего30324
read, ДО нового accumulate/latch. Его held/pressed относятся к предыдущему
завершённому latch. Exact ROM305E0 вычисляет99AC и копирует в48EC;300C4
выбирает контроллер и копирует48EC вC69C. BB730 выбирает port0; scene1
E4484 читает48EC напрямую, Start ставит80218B00 и снимает paused802183CA.
В061 нет значений этих полей непосредственно в момент чтения E4484.
Следовательно, текущие журналы не доказывают конкретную причину отказа.

Удачные сессии1/2 после world-scripted Native acquire исполнили skip-thread7
802183C0,3C9C/DDC9C/DDB20 и перешли в scene34 Intro. В session4 после
acquire15390 исполнялись обычные сценарные команды DEE70/E233C; перехода
к scene34 нет. Это локализует наблюдаемое расхождение к начальной заставке,
но ENTER — только первый вызов функции в потоке; это НЕ счётчик обновлений.
Нельзя утверждать, что interpreter остановлен или Start не попал в него.
Рендер/PCM доходят до task2334. SHA256 журналов и сводки сессий:
[research/input-loss-after061.json](../research/input-loss-after061.json).

## Новая диагностика в исходниках

Observer schema2 добавляет latched48EC, selected39E0/9AD0/C69C,
аккумулятор44E0/history9548, process slots/status/phase/callback, Source
tick/VI divider/retrace/count/built/deferred и Native owner/FPS. Для реально
загруженной scene1 — pause/transition/room/demo/scripted/start_requested;
для её активного interpreter — восемь script records и bounded current opcode.
Source dispatch800281B8/phase0, world callbackBB730, script inputE4484,
scene34 callbacks и Source completion получают отдельные change caches и
visit counters. Счётчики/время/меняющиеся waits пишутся в готовый снимок,
не отменяя dedup; heartbeat каждые300 посещений отдельно по каждой точке.
Это позволит отличить отсутствие Source updates, перезапись кнопки и
отказ уже выставленной команды пропуска. Simulation/extra draw не считаются
Source. Никаких записей в RDRAM или изменения управления/сценария нет.

Все hooks используют существующие host trace/native_ui_source boundaries;
canonical startup-menu-reset-v1 не изменена, новая CPU generation не нужна.
Две итоговые host units startup_runtime/native_ui скомпилированы Linux
C++20, exit0; точные source hashes в compile receipt и research выше.
На момент первоначального анализа Windows EXE/link/package не запускались;
затем по прямой команде пользователя собран/упакован062 (см. запись выше).
Тесты и game/emulator/renderer/GPU не запускались. Этой диагностики НЕТ в061,
она присутствует в062. Новое правило разрешает билд сразу после изменений кода.
Корневая причина и fix НЕ объявлены установленными. Commit/push не выполнены
и не разрешены до пользовательской проверки соответствующего исправления.
