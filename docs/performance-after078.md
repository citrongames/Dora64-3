# Результат078 и уточняющий профиль079

Источник logs/1791473499098/state/graphics-profile.jsonl, SHA256
4dd59eb24823ebbcf194af19900c36aadecf941b9b5c3e1fea5f55bb6541b04f.
82 агрегата, scope_errors0. Пользовательский запуск, агент игру не запускал.
Агрегаты стабильных окон: research/test078-graphics-profile-analysis.json.

| Участок | FPS | Producer wall/кадр | HLE self | Tiles | Producer GPU wait |
|---|---:|---:|---:|---:|---:|
|2D149,5–6с|360.14|1.162|0.042|0.056|0.396|
|Intro376,8–24с|116.96|7.521|2.692|2.659|0.915|
|Тяжёлая165,48–83с|86.91|8.310|2.835|3.950|0.785|

Единицы длительностей — мс/guest frame. HLE inclusive содержит fullSync;
self исключает измеренных детей. Разные потоки перекрываются. Native update
в1652.912мс/frame и build1.990 также остаются, build включает blocking submit.
Мелкие thread CPU измерения Windows ненадёжны по отдельности; cycles/wall
используются как дополнительные признаки работы против ожидания, не как частотаCPU.

В1653149deferred,0eager и0materializations: постоянный framebuffer fallback
исключён в этом окне. Intro намеренно имеет request_off/not_gameplay и eager.
Texture stage включает повторное исполнение load operations, хэшированиеTMEM,
lookup/upload. Один078 timer не отличает их. Нельзя объявлять только hash или
только байтовую копию причиной всех3.95мс. Просмотрены исходникиTMEMHasherv5,
RDP::loadToTMEMCommon и RSP::setVertexCommon/drawIndexedTri/State::loadDrawState.
Изменять их алгоритм без уточнения распределения затрат пока не стали.

079 только добавляет выборку1/64 вызовов: TMEM load, deferred load planning,
TextureManager uploadTexture и чистый TMEMHasher::hash; vertex batches,
triangle, matrixCommon, loadDrawState, flush. Пишет observed_calls и calls:
суммарное время sampled stage НЕ равно всей стоимости функции. Для оценки
использовать mean measured duration * observed calls; overlap/nesting сохраняются.
Счётчик каждого вида перемешивает ordinal, чтобы не совпадать с фазой кадра.
Новых per-command logs нет, грубые078 scopes остаются.13 translation units.
Профиль schema2, тот же logs/<run>/state/graphics-profile.jsonl.

Никаких изменений gameplay/AI/contacts/time/catch-up, графических результатов,
texture cache semantics, RAM readback policy и queues/waits. Dependency sources
не редактируются: pinned build-local copies RSP/RDP/TMEM через адаптер. Tests,
CPU checks, игра/эмулятор/renderer/GPU агентом не запускаются. Git не выполняется.
Пользователю достаточно15–20сек начальной катсцены и15–20сек тяжёлой165 сцены.
Это уточняющий профиль, не FPS исправление. После исправления вернуть clean.


## Сборка079

MSVC RelWithDebInfo /O2 exit0,28.797с. EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-079.exe.
SHA256 a53e34f78ec20f438b40e8f7f961cad5c53228a94c32ab8e9025ab44fca4164e.
13 инструментированных translation units; общие diagnostics/profile выключены.
Все249 старых файлов внеlogs сохранены size/mtime;DLL проверены упаковщиком.
Игру/тесты агент не запускал, Git не выполнялся. Для следующего замера достаточно
20–30сек в той же тяжёлой сцене; повторять все режимы не требуется.
