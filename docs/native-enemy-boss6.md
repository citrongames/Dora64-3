# Boss6 454–471

All eighteen types and 103 reviewed Source functions are included. Twelve-state
head, independent body/leg phases, rockets, four fannels, laser variants, fire,
smoke, bomb, shot, flashes and crash retain their real callback/ROM identity.
Native equations cover speed/vertical ramps, circular radius/phase, sine turns,
countdown/parity height, approach/accumulated angular velocity, scalar opacity,
shared scale, view angles and original velocity projection. Model time and exact
frame cues use Source clip ownership; teleports, shell/HP, RNG, probes, links,
spawns and exit/reward commands retain original order.

Head reads the detached body's preceding motion, not its future update.
Six-field ghost delay retains immutable equations through each actual Source
queue store. Attached parts copy exactly the Original fields; laser pure matrix
prefix stops before FSM, fannel orbit keeps its own height offset and wave.
Parent selectors and lifetimes are validated; no pose path replays AI.

Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).


ArmFlash459 копирует parent yaw лишь в state1; state10 сохраняет конечную
позу до unlink. Fannel467 при переходе1→4 ещё сохраняет Source aim; orbit
attachment начинается после реального входа в4. Diagnostics30 root454 и
последующий полный218-kind replay проверяют этот порядок без renderer.

## test144: повторный бой с роботом / Boss6, Manual FPS

Пользователь сообщает: ступенчатая орбита головы при трёх частях,
вертикальное дрожание формы с лазером, остановка финальной головы с длинной
полосой рядом. На Original в этом бою ещё не проверял; расследование ведётся
как регрессия Manual. Исходная логика Original оставлена без изменений.

Статически установлены и исправлены четыре ошибки native_enemy.cpp:

- ROM111E70:24548 хранит угол f0, а адаптер читал f20, уже занятый double
  radians. Из-за этого захват фазы не совпадал с RAM и орбита оставалась
  дискретной. Теперь радиус, координаты и видимый угол используют фазу f0.
- Копирование Y корпуса в голову25000 при равенстве текущей высоте — no-op.
  Раньше адаптер повторял уже завершённое движение корпуса, отматывая голову
  вверх перед собственным спуском. Равное присваивание больше не превращается
  в historical_axis; настоящий перенос высоты отдельного корпуса сохранён.
- Финальный рывок26E08: Source умножает скорость на ROMdouble80228C18=1.5.
  Общий обработчик exponential Store не заполнял retention (оставался0):
  дробная скорость становилась0, проекция использовала log(0), что может
  дать нечисловые координаты и выключить обработку актёра. Таблица теперь
  содержит ROM-коэффициенты, общий обработчик их читает. Тот же пропуск
  обнаружен у Boss5 effect448/DA2C (double80221990=1.1); исправлен вместе.
  Все exponential Store требуют адрес коэффициента при компиляции.
- Final26E78 использует -atan2(Y-80,radius). Общий spatial_pitch использует
  target-current: знак дробного наклона был противоположным Source. Для
  этой операции задан нужный знак; наведение Fannel не меняется.

Адреса SWC1 и коэффициенты сверены с ROM; evidence:
research/boss6-refight-after143.json. Зависание и полоса согласуются с ошибкой
проекции рывка, но подтверждение устранения требует игрового повторения.
Не добавлены таймауты, принудительные переходы фаз или удаление частей босса.

Одна инкрементальная Windows Release144 без CrashLog/GraphicsProfile/Diagnostics.
Генерация startup-fire141-v1 повторно используется: меняется только C++ адаптер.
Исправления141/142 сохранены. Игра, тесты, CPU checks, эмулятор и renderer/GPU
агентом не запускались. Сборка и игровое подтверждение — разные проверки.
Коммит/push — только по новой команде владельца после проверки.


## test146: журнал145 разобран, дрожь формы и подъём шипов

Владелец подтвердил устранение софтлока144; оставалась постоянная дрожь
двухчастной формы. Журнал145 содержит11621 BOSS6FORM строк. Во время атаки
state7/phase7 высота головы во всех7896 отсчётах равна80, кадр модели60.
Значит, предположение о вертикальной анимации не подтвердилось.

ROM25794..258BC каждый шаг возвращает X/Z на заднюю направляющую:
X ограничен ±700; Z=max(abs(X)-500,0)-850. Затем добавляется скорость20.
Например, X/Z сбрасываются в(-700,-650), после шага становятся
(-708.530823,-631.910645). Manual показывал промежуточный уход и возврат
каждого шага. Теперь именно в state7/phase7 поправка этой направляющей
распределяется вместе с движением за тот же период. При взаимной компенсации
позиция стоит; при перемещении к следующей точке движется плавно. Конечные
координаты, скорость, направление, атаки и исходные условия остаются Source.
Поправка сбрасывается при внешней команде движения. Первый вход и прочие
состояния не обрабатываются как поправка направляющей.

Шипы — Fannel467, state4. Исходный218F8 прибавляет к синусоиде высоту
родителя минусphysical60. Прежний адаптер использовал высоту родителя как
постоянную на период, затем перезаписывал ею плавное следование. Теперь
уравнение включает неизменяемую траекторию родителя плюс собственную волну.
Точный Source-результат конца периода сохранён; X/Z, стрельба и фазы не меняются.

По прямой просьбе владельца следующая сборка146 БЕЗ журналов:
Release, CrashLog/GraphicsProfile/Diagnostics OFF, без PDB/CodeView.
Игра/тесты/CPU checks/GPU/эмулятор не запускались. Изучены журнал и ROM,
выполняется одна инкрементальная сборка. Игровое подтверждение ожидается.
Evidence: research/boss6-test145-review.json. Коммит/push без новой команды нет.
