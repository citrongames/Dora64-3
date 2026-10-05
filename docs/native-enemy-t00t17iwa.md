# T00 shield354–356, T17 door357–358, Iwa392

В основную runtime включены полные Source closure статических контроллеров,
их связанных эффектов и конечных замен, а также Iwa. Настоящие модели получают
Native clock; прогресс, столкновения, animation-end queries, birth failures,
родительские команды, unlink и retirement сохраняют Source порядок.

Effect355 копирует parent Y+180 только при входе. Непрерывного parent-follow
в исходнике нет; он не добавлен. Identity parent7C и shield354 child slot0
учтены среди командных зависимостей, без ложной geometry attachment.

25 функций получили смысловые имена, описания и доказанные scoped variables.
Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
