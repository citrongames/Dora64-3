# Metabo322, Gabugon323 и tongue324 в составе блока2

Внесены полные Source closure трёх связанных owners: roaming/capture Metabo,
родитель Gabugon, настоящая tongue324, movement/bite/вознаграждения и model
animation. Capture points и pickups births сохраняют Source команды/порядок.
Metabo получает exponential XZ paths, Y-2 и Source X→Y shrink; tongue получает
exponential extension/retraction и реальные X→Y/Z scale copies.

Tongue state1 следует настоящей позе родителя через reviewed geometry-only
boundary с guard slot/lifetime. State2 замораживает позицию исходно; следование
не добавляется туда произвольно. Parent slot0 child lifetime входит в identity.
17 функций получили смысловые имена и описания. Общая generation, compilation,
CPU tests и Windows build ждут завершения всего блока2.
