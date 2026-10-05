# Исходящие зависимости всех врагов блока2

Полностью прочитаны101 direct sharedtargets из1155 private Source closures;
62 ранее неописанных helper получили смысловые имена с точной ROM/ABI identity.
Source scopes сохраняют регистрацию ctx; имена не меняют таблицы callbacks.
Разобраны duration/frame/seek/flags/channels, RNG, probe/gravity/reflection,
damage/health/progress/room/dialogue, linked lifetime, births/retirement, sound
и общие effect producers. Уже описанные39 sharedtargets сохраняют каталог.

Добавлен реальный model owner478 BroboA100Break с тремя Source functions в
level1_slot2. Другая функция8023D944 изremaining_slot2_04 не имеет его ctor/contact
pair: это address collision, не второй обработчик478. Native не связывает их.

FX producers выполняются только на Source boundary; их реальные общие effect
scripts801FC2E0/801FC33C имеют реальные kinds257/258, а следующий script
использует259. Все три owner включены в native-enemy-sharedeffects.md;
ранее предположенная связь с129/130/131 исправлена по ROM evidence. Границы этой связи войдут в итоговые Source fixtures.

Семейство включено в полный218-type блок. Генерация362 и проверка имён
завершены; весь218-kind replay и28 connected CPU checks PASS043. Границы описаны в
[native-enemy-fixture-coverage.md](native-enemy-fixture-coverage.md).
