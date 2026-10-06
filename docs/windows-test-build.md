# Test-047: исправления FPS после046 и общий аудит

Windows EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-047.exe,
generation startup-regressions-test047-v1.
SHA256: a54022fed189e03a9234aaea62d6a1ec3bea109ed5829ab89ed90a38c0f176d8.

Общий механизм плавного сдвига authored byte format2/CI4 textures, включая воду:
распознавание точного переноса пикселей с той же палитрой, дробный RDP tile
origin и копии display lists в текущем frame bank. Найдены11 ROM семейств /
94 кадра; реальная достижимость каждого кадра не заявляется. Для10 семейств
найдены static ID tables; один набор пока без доказанного владельца.

Исправлены Native eligibility fades и игровых menus, матрицы моделей выбора,
дробные координаты stage-map sprite и зарядка оружия в RDP10.2. Выравнен
момент Source завершения для модели, меню, миниигры и текстуры. Проверены
аналогичные пути:4 viewport origins гонок/самолётов, постоянные копии моделей,
followers после terrain query, тени участников, terminal retreat и camera
distance rates. Source рождения эффектов/команды/score/input/RNG сохранены.

31 завершённая CPU проверка PASS; общий набор PARTIAL_USER_SKIP:
часовой full-player исключён по прямой команде пользователя. Выполненные
сценарии объектов сохраняют исходное execution provenance; не весь набор
запускался повторно. Игра/renderer/GPU локально не запускались.

Реальный визуальный результат всех сообщений ещё проверяет пользователь.
Соответствие voice/fruit конкретной миниигре и причина камеры у стен не
подтверждены визуально. Неизвестные auxiliary/loading/framebuffer пути
сохраняют Original handoff; новые кадры различающегося bitmap art не создаются.
Прежние EXE/ROM/settings/saves/DLL и donor/dependency pins сохраняются.
Имена и адресные соответствия декомпилированного Source обновлены.
