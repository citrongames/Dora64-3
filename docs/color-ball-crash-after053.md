# Вылет при повторных касаниях цветных шариков после053

2026-10-07. Пользователь подтвердил исправление выхода через гаджет-дверь,
затем сообщил вылет после нескольких касаний шарика, выдающего цвет.
На старте main c641dd1, дерево чистое; последний EXE053.

## Фактический вылет

Дамп: C:/Users/black/AppData/Local/CrashDumps/Dora64-3-test-053.exe.11472.dmp.
Журнал: D:/Games/DoraRecomp3/testbuild/logs/1791384974142/runtime.log.
EXE053 SHA256 b6d32cf7f8421d331e9fbe3682c3e4750817ad7122d048cc43262a8a1064a235.
LLDB прочитал дамп offline, игра не запускалась. Access violation C0000005,
RVA62970A, dora3_sound_color_sequence_append_8023D730+13A, чтение byte по неверному адресу.
Caller chain:8023E020 ->8023E334 (SoundLightBlue/Red/Yellow update), через
native scene/enemy callback и native gameplay update. Это не FadeTimeline.

Соответствующая ROM инструкция8023D7B8 читает очередной цвет эталонной строки.
Source helper принимает цвет0/1/2, сохраняет его в буфер802401A1..A3, увеличивает
count802401A4 и обнуляет timeout802401A8. При count3 сравнивает три цвета с
одной из6 строк по3 байта в таблице8023FCA0; индекс строки в802401A5.
Результат802401A6 получает1 при совпадении или-1 при первом несовпадении.
ROM содержит все6 перестановок цветов. Body200 bytes SHA256:
4c22dc09208a87d613f1469cb6bb8e72614ba17a22ccba01958c85b0c542044d.

## Расхождение с машинным кодом Windows

ROM/C вычисляют row=0x8023FCA0+3*index; при index1 адрес8023FCA3.
В EXE053 MSVC19.44 /O2 сделал другое:
movsbq(index), add -0x3FEE01B0, lea (rcx,rcx,2).
То есть low32(3*(index+0xC011FE50)). В дампе rcx=C011FE51 до lea,
r11/r3/r6=4035FAF3; это соответствует index1 и неверной row4035FAF3.
После XOR3 и RDRAM bias получается смещениеC035FAF0 вместо valid offset23FCA0.
Исходные MIPS shift/add и C ADD32 не умножают адрес таблицы на3.
Неверное чтение происходит только при третьем принятом цвете; последние
строки audio/RT64 в журнале не определяют причину.

## Исправление

tools/msvc_codegen_workarounds.py оборачивает только этот200-байтный guest helper
в MSVC pragma optimize off/on, сохраняя тело и восстанавливая настройки после
функции. Остальные функции сохраняют прежнюю оптимизацию; Clang/GCC не затронуты.
Проход включён в generate_startup до смысловых имён. Overlay identity, адрес,
размер и полный ROM hash обязательны; отсутствие/дублирование C body — ошибка
генерации. Сам модуль включён в generation receipt inputs.

Функция названа dora3_sound_color_sequence_append_8023D730 в каталоге;
старый ABI сохранён. Генерируемый игровой C вручную не редактировался,
библиотеки и их pins не менялись. Логика цвета/касания/успеха головоломки,
звуки и частота Source commands сохранены.

## Компиляция и границы

Новый генераторный проход применён к отдельной build-local копии настоящего
funcs_93.c. Весь этот translation unit успешно скомпилирован MSVC с исходными
/O2 /Ob1 /std:c17 и /FAs; без link, выполнения и синтетического fixture.
Просмотр assembly подтверждает: r2 <<1, затем прибавление r2, затем ОДНО
прибавление r4=8023FCA0 с32-битным sign extension. Ошибочная свёртка устранена.
Материалы: build-local/color-ball-crash053/{compile.log,sequence-function.asm}.
Это только компиляция и статический разбор, не runtime PASS.

Тесты, игра, эмулятор, renderer/GPU не запускались. По прямой команде пользователя
правка5b1bf19 включена в Windows054; новая CPU generation startup-color-ball-test054-v1,
RSP audio-rsp-v1 прежний. MSVC RelWithDebInfo сборка успешна.
EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-054.exe.
SHA256: 21987e5020ddabd648cdff8a8f6c1ef425aa8f4f83eaa8af7a736a8e4f617f5b.
Все174 прежних файлов вне logs сохранили хэши; PDB053 сохранён в symbols-test053.
Пользователь подтвердил исчезновение вылета в054. GitHub возвращает Internal Server Error при push;
локальные коммиты сохранены, но отправка ещё не подтверждена.
