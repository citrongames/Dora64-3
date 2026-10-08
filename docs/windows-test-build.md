# Test-065: восстановление и заморозка HP

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-065.exe.
Исправлен backend Infinite HP из064: чтение собственного максимума
персонажа теперь использует правильное знаковое расширение адреса.
HP выбранного персонажа заполняется и замораживается по его Original
максимуму; отдельный progression flag доступности не блокирует чит.

PC menu: Escape / Back–Select → Cheats → Infinite HP.
Восемь переключателей и прежние семь читов предметов с99064 сохранены.
Режимы Original20/30, Display и Native/Manual используют общий helper.
Reset/input/cutscene fixes063 сохранены; settings/saves/ROM и старые EXE/DLL
не переписываются. Сохранённые пользователем переключатели остаются действующими.

Только инкрементальная компиляция и упаковка; тесты/CPU checks/игра/
эмулятор/renderer/GPU агентом не запускались. Игровой результат065
подтверждён пользователем сообщением «работает, коммит и пуш»; разрешено
сохранение проверенного этапа в main. Compile/package receipts и README
рядом с EXE остаются исходными записями сборки; QA записана отдельно в
research/windows-test065-user-confirmation.json.
Подробности: docs/hp-cheat-after064.md.
