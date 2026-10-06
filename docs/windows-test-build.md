# Test-046: итоговая интеграция FPS

Windows EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-046.exe,
generation startup-integration-block6-v1.
SHA256: 2dfdf6492037426c1f54c4312cae1e844e6b8124ef07a7a8abb6d16e360e5526.

Весь выбранный блок6: часы после реального входа в Native, точная принадлежность
сессии callbacks/ROM/divider/load-generation, общий frame timestamp, безопасное
дренирование обоих банков и stale wakes. Возврат в Original сохраняет дробную
фазу моделей и исходные Source решения/cues без потерь и дублей; pause/seek,
reload/reset и foreign writes имеют отдельные границы. Резервные player states
сохраняют Native мир; Pak UI получил Native движение курсора/размеров/подсказок,
device I/O и authored blink/sprite cadence остаются Source once.

30 завершённых CPU checks PASS, включая новый actual common loop integration.
Долгий full-player прерван по прямой команде пользователя; PASS ему не присвоен.
Общий pre-build не завершён полностью. Синтетические CPU inputs не заменяют
прохождение/визуал.
Вспомогательные actor/resource/model экраны и unsafe/unknown/loading/framebuffer
границы остаются явными Original fallback; их Native движение не заявляется.
Игра/renderer/GPU локально не запускались. Прежние файлы, ROM/settings/saves/DLL
сохраняются при упаковке. Donor и dependency pins не менялись.
