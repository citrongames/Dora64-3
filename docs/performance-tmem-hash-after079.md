# После079: повторное хэширование TMEM

Пользовательский профиль1791474301346, SHA256
 d40236b6a05c9ee3e062c4aa18a2cffceee7a7dc78386b343e3e5390a5a21782.
41 samples, scope_errors0. Stable165 (eager0/deferred>0):26samples,
2169guest frames,82.919FPS, producer8.658мс/frame, tiles3.982мс/frame.
Точные aggregates: research/test079-graphics-detail-analysis.json.

## Что измерено и что установлено по коду

TextureManager::uploadTexture вызвана1288097раз в выбранном окне; measured
20127samples. Вложенный TMEMHasher::hash имеет126.034мс sampled суммарно,
6.262мкс/измеренный вызов; observed/calls scaling даёт3.719мс/frame. Эта оценка
содержит стоимость observer и не является точными3.719мс полезной работы.
Нельзя складывать с inclusive uploadTexture или HLE: nested/parallel regions.
LoadToTMEM sampled оценка0.935мс/frame также включает observer; сумма может
превысить непосредственно измеренный texture stage. Тем не менее хэширование
выделено как крупный расход, существенно дороже отдельных load operations.
Triangle/vertex sampled estimates сильно чувствительны к observer overhead;
на их основании геометрические алгоритмы не менялись.

Pinned TextureManager::uploadTexture всегда вызывает TMEMHasher::hash перед
поиском hashSet. Наличие текстуры наGPU не позволяет избежать повторного
вычисления. V5 приTLUT проходит индексы пикселей, собирает набор используемых
цветов, хэширует выбранные palette entries и параметры. Повторное одинаковое
содержимое до080 не запоминалось. Это подтверждённая вычислительная работа
в измеренной горячей функции; её доля, устранимая cache hits, ещё не измерена.

## Исправление080

src/rt64_tmem_hash_cache.hpp кэширует только чистую функцию hash приtlut!=0
иversion5. Ключ включает width,height,tlut,version,line,siz,fmt,tmem,palette —
все поля, читаемые pinned TMEMHasher/needsToHashRowsIndividually. Сохраняется
полный4096byte TMEM snapshot. Быстрый XXH3 fingerprint только выбираетbucket:
cache hit требует также равенства ключа иmemcmp всех4096bytes. Коллизия
fingerprint не может вернуть чужой результат. Изменившаяся texture/palette,
размеры/формат/stride/address вызывают исходный TMEMHasher. Не используется
адресRDRAM как признак неизменности и не пропускаются load/TLUT operations.

Cache ограничен512buckets*4ways=2048entries (~8.2MiB), LRU вbucket, лениво
выделен отдельно каждому producer thread. Не содержит указателей на игровые
ресурсы/GPUobjects. Поэтому reset/scene switch не требует инвалидировать
неизменяемую функцию от bytes: прежняя запись полезна только при exact equality.
GPUhashSet lookup,insert,queueGPUUploadTMEM,texture dumps/removeHashes/eviction
выполняются в прежнем порядке. GPU eviction по-прежнему приводит к нужной
загрузке даже при попадании в новый кэш численного hash. Non-TLUT и другие
версии используют исходную функцию; rawTMEM/uploadEmpty не затрагиваются.

Адаптер tools/rt64_tmem_hash_cache.py работает после существующего генератора
профиля с build-local rt64_rdp_tmem.cpp; dependency checkout/pins read-only.
Дополнительно pinned TMEMHasher header SHA256
3267cd4a85c61e1a960df175eb64641a75dceccb8d38e680bb0fcf85912d15c5:
изменение алгоритма останавливает генерацию и требует пересмотреть ключ.
Замена касается только вызова hash, весь последующийGPUcache код сохраняется.

## Границы результата

После исправления — чистый080: GraphicsProfileOFF/DiagnosticsOFF, generic
PROFILE/traces disabled. Cache hit rate и FPS080 агентом не измерены; локально
игра/эмулятор/GPU/CPU checks/tests не запускаются по инструкции пользователя.
HLE geometry, Native updates и другие расходы остаются;300/360FPS не обещаны.
Игра/AI/contacts/time/catch-up/animation boundaries,077readback policy и waits
не меняются. Компиляция подтверждает только сборку. Commit/push по команде.


## Сборка080

MSVC RelWithDebInfo /O2 exit0,25.844с. EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-080.exe.
SHA256 5a4a229b96c4bec2d428244eaee8fd438f520542277904b41a432da10a06a707.
DIAGNOSTICS/GRAPHICS_PROFILE=OFF, generic PROFILE отсутствует. Все252 старых
файлов внеlogs сохранены size/mtime; общиеDLL проверены упаковщиком.079EXE/PDB
и receipt сохранены. Игра/тесты агентом не запускались, commit/push не выполнялись.
