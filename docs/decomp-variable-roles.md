# Роли переменных в декомпилированном C

Регистр MIPS не равен одной исходной переменной: он может сменить роль в той же
функции. Каталог назначает имя точному доступу `ctx->rN` только в подтверждённой
области инструкций `[start, end)`. Входящий регистр, сохранённый caller state,
новый аргумент вызова и восстановление регистра остаются за этой областью.

Генерируемые локальные псевдонимы раскрываются в те же register lvalues.
Запись по понятному имени обновляет прежний слот контекста; новых копий значений
нет. Имена очищаются после функции. Точные интервалы, инструкции и основания
хранятся в `recomp/decomp-symbols.json` и выводятся рядом с определением функции.
Для неизвестных значений сохраняется регистровое имя.

Например, в полном player update `controller_index_r16` означает индекс
контроллера. В функции изменения здоровья `health_delta_r4` — входная прибавка
к здоровью до переиспользования r4 как status pointer. Эти роли нельзя переносить
на весь регистр или на другой overlay.

| Имя | Исходный доступ | Подтверждённая роль | Число областей |
|---|---|---|---|
| `actor_opacity_physics_r16` | `ctx->r16` | Saved actor physics pointer through the model opacity call and byte2A update | 1 |
| `actor_physics_r9` | `ctx->r9` | Physics record retained while unlinking peer lists | 1 |
| `actor_pool_index_r5` | `ctx->r5` | Текущий индекс диапазона пула; инкрементируется после каждой записи pause flag | 1 |
| `actor_pool_index_r7` | `ctx->r7` | Actor pool index compared with peer backlink entries | 1 |
| `animation_command_r5` | `ctx->r5` | Входной номер кадра для seek либо команда0/1/переключить для флага. | 3 |
| `animation_model_r4` | `ctx->r4` | Входной handle модели; не меняется в этой leaf функции. | 3 |
| `arizinogu_actor_r16_8022DD68` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r16_8022E174` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r16_8022E674` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r2_8022E6D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r2_8022EB80` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022D778` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022D944` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022DA4C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022DC7C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022DD10` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022DDF8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022E07C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022E7FC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022EA24` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022EA9C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r3_8022EDC4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r4_8022E3B8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r4_8022E78C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_actor_r5_8022E1C8` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022D790` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022D808` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022D93C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022D988` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DA90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DC94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DD08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DE44` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DEE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DF38` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022DFE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E084` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E0D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E15C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E258` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E36C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E428` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E5C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E654` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E724` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E770` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E7CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E804` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022E85C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022EA40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022EA94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022EC18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r2_8022EDEC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022D7E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022D844` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022D89C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022D8D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022D960` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022DA70` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022DAD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022DCA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022DCBC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022DE14` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E0B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E140` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E208` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E26C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E30C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E400` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E570` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E6F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022E888` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022EA50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022EA68` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022EB94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022EC5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r3_8022ED64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022D780` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022D7A0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022D9F0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022DC84` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022DE7C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022E1B0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022E6B0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022EA2C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r4_8022EDAC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `arizinogu_physics_r5_8022EAD0` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `axis_rotation_angle_float_bits_r5` | `ctx->r5` | Float-биты угла до переноса в f20; затем a1 становится указателем компоненты оси | 1 |
| `axis_rotation_output_matrix_r16` | `ctx->r16` | Выходная матрица между сохранением a0 в s0 и восстановлением старого s0 | 1 |
| `axis_rotation_x_float_bits_r6` | `ctx->r6` | Float-биты осиX до сохранения в scratch-аргумент нормализации | 1 |
| `axis_rotation_y_float_bits_r7` | `ctx->r7` | Float-биты осиY до delay-slot сохранения перед нормализацией | 1 |
| `baguram2_actor_r2_8021E754` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r2_8021ED94` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r2_8021F378` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r3_8021E42C` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r3_8021E79C` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r3_8021E9A8` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_actor_r4_8021F3E8` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E3F8` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E47C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E5A4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E6EC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E7F4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E828` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021E884` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EA60` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EAAC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EB1C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EB3C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EBA4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EC14` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EC74` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EDE8` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021EE18` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F044` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F090` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F100` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F120` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F188` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F1F8` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F258` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r2_8021F3C4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021E450` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021E4C8` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021E5CC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021E7C4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021E960` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021EA40` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021ED48` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021EDBC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021EF8C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021F024` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021F32C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r3_8021F39C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r4_8021E408` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r4_8021E784` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r4_8021E990` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram2_physics_r4_8021EE54` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_actor_r2_8021F634` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_actor_r2_8021F6A4` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_actor_r3_8021F540` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_actor_r3_8021F758` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r2_8021F50C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r2_8021F594` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r3_8021F564` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r3_8021F600` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r4_8021F51C` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r4_8021F68C` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram3_physics_r4_8021F760` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r2_8021DA58` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r2_8021DDF8` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r2_8021E154` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r2_8021F9FC` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r3_8021D7BC` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r3_8021DAA0` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r3_8021DCB8` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r3_8021F8C8` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r4_8021DA28` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_actor_r4_8021E22C` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021D788` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021D808` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021D8DC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021D9E4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021DAEC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021DB44` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021DE5C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021E208` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021F8E0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021FA58` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r2_8021FB7C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021D7E0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021D84C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021D8C4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021D904` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DAC4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DBE8` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DC54` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DD44` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DDAC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DE2C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DF3C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021DFAC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021E010` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021E0A0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021E108` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021E1E0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021F8F0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021F908` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021F934` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021FA34` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r3_8021FAD0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021D798` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021DA88` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021DCA0` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021E170` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021F8D0` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r4_8021F980` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `baguram_physics_r6_8021DE94` | `ctx->r6` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r2_80221C0C` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r2_80221C5C` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r2_80221D34` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r2_80221E58` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r2_802220D4` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r3_80221B24` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r3_80221DBC` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r3_802220F4` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r4_80221F48` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_actor_r4_802221A4` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r16_80221CAC` | `ctx->r16` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_80221B30` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_80221B6C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_80221E74` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_80221F00` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_802220EC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_80222140` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r2_8022222C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221B88` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221C84` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221D1C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221DE4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221E44` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80221E60` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80222080` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_802220DC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80222118` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_80222204` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r3_802222B0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r4_80221B40` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r4_80221C64` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r4_80221CD0` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r4_80221D3C` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu2_physics_r4_80221D9C` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_80220F18` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_80221008` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802210E4` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802211CC` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802211FC` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802214D4` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_80221508` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802215A8` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r2_802219F8` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r3_80220E28` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r3_8022105C` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r3_802224B0` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r3_8022262C` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r3_80222A18` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r4_8022133C` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r4_80221658` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r4_80222528` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_actor_r4_802226F8` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r16_8022197C` | `ctx->r16` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r16_8022257C` | `ctx->r16` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80220E34` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80220E70` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80220EB4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80220F50` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80220FD0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_8022104C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_8022109C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80221248` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_802212F4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80221584` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_802215F4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_802216E0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_802219E0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_802224CC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80222640` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_8022269C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_8022272C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80222770` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80222920` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r2_80222A40` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80220E8C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80220ED4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_8022102C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80221078` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_8022111C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80221198` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80221220` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_8022149C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80221510` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_802215CC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_802216B8` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80221764` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_802224DC` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_802224F4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80222520` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_80222804` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_8022286C` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r3_802229B8` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80220E44` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80221010` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80221204` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80221268` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80221590` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_802224B8` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r4_80222A00` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `barupu_physics_r5_802228A0` | `ctx->r5` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r17_8021EBB8` | `ctx->r17` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r17_8021EC08` | `ctx->r17` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021D978` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DB2C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DC38` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DCA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DDBC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DEF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021DF60` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E050` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E4CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E534` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E714` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E77C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E7D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021E868` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021EBE0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021EC84` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021ED30` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021EE4C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021EE74` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F20C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F61C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F6E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F764` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F7CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021F8E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021FA20` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021FA88` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021FB78` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_8021FFDC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_80220044` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_802200A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r2_80220134` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021D7A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021D81C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021D9A8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021DAFC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021DBB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021EAA0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021F134` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r3_8021FCA0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021DA8C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021DDE8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021DE1C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E084` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E0D0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E178` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E3D0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E46C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E580` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E5C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E624` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E6A8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021E818` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F1B0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F29C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F4D8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F5B4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F650` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F910` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021F944` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021FBAC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021FBF8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021FEE0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_8021FF7C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_actor_r4_802200E4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021D848` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021D9B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021DD58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021DFA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021E198` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021E570` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021E7B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021EB7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021EED0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021EFB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F188` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F1B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F2DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F35C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F3A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021F880` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_8021FAD0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r2_80220080` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021D7BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021D7D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021D90C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DA00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DA34` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DA94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DBEC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DCBC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DD38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DD78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DDA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DDE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DE24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DEAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021DF7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E058` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E07C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E0C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E214` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E2D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E31C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E378` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E3C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E464` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E550` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E5BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E62C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E6A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E6C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E794` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021E820` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021EB10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021EB2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021EB58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021ECCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021ED48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021ED70` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021EF88` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F018` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F14C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F164` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F248` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F378` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F464` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F4E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F528` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F55C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F5BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F658` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F7E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F860` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F8A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F8CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F908` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F94C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021F9D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FAA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FB80` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FBA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FBF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FD24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FDE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FE2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FE88` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FED8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_8021FF74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_80220060` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r3_802200EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021D814` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021DAE0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021DB34` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021DBA0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021DBD8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021DCE0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021EC8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021ED38` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021F600` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021F6C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021F704` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021F808` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r4_8021FC8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021D874` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021D8E0` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021DFE4` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021E164` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021E1D8` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021E29C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021F3CC` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021F438` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021FB0C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021FCE8` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `batti_physics_r7_8021FDAC` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r16_8022EA1C` | `ctx->r16` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r16_8022EAB8` | `ctx->r16` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r16_8022EC44` | `ctx->r16` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r16_8022ED18` | `ctx->r16` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r16_8022ED90` | `ctx->r16` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022E890` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022E9FC` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022EA98` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022ECC8` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022ED40` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r2_8022EE08` | `ctx->r2` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r3_8022E6B4` | `ctx->r3` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r4_8022E7D8` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r4_8022E8F8` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r4_8022EE70` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r4_8022EFE8` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r4_8022F09C` | `ctx->r4` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_actor_r5_8022F1BC` | `ctx->r5` | Указатель actor из global8021BDEC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r17_8022E93C` | `ctx->r17` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022E6CC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022E7B8` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022E7E0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022ECFC` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022ED74` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022EFE0` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022F3C4` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r2_8022F41C` | `ctx->r2` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022E798` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022E818` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022E8B4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022E8E4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022EB18` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022EC24` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022EDE4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022EFB4` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F010` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F3A0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F460` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F4A0` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F510` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r3_8022F540` | `ctx->r3` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r4_8022E6BC` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r4_8022E898` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r4_8022E9E0` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r4_8022EA7C` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r4_8022EE10` | `ctx->r4` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r5_8022EBC8` | `ctx->r5` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r5_8022EE68` | `ctx->r5` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bironn_physics_r7_8022F180` | `ctx->r7` | Указатель physical из global8021BCDC; роль ограничена до следующей записи регистра/внешнего вызова | 1 |
| `bobbing_actor_r16` | `ctx->r16` | Actor retained across the sine and angle normalization calls | 1 |
| `boss1_actor_r16_8021DE54` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r16_8021DEE4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r16_8021F968` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r16_8021F988` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E0CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E25C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E284` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E2E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E3A8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E3D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E4EC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E704` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021E96C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021EA34` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021EAE8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F0F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F200` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F300` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F39C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F3F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F614` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F764` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F804` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021F858` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_8021FAE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_802200CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r2_802205FC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021DBFC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021DE44` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021DED0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021E0F4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021E1D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021E798` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021F418` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021F5A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021FB44` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021FC34` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_8021FF10` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_802204A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_802205A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_80220614` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_802206FC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_80220794` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r3_80220858` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021DF3C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021DF64` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021DF9C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021E884` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021E940` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021EF1C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_8021F0B0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_80220034` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_80220184` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_actor_r4_802207BC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r16_8021EFD8` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021E790` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021F094` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021F144` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021F4EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021FD88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8021FF08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_80220028` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_80220134` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_802202F4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_80220490` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8022059C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_8022079C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_802207C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r2_80220884` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021D91C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DA28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DAFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DBD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DC18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DC5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DDF8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DEFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021DF18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E114` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E528` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E670` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E6C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E730` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E860` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E918` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E97C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021E9B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021ED28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021ED7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021EE58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021EF94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F014` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F324` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F394` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F4CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F5B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F638` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F7E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021F888` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021FBA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021FD08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021FE34` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8021FEDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220048` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220104` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220160` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220294` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220334` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_8022046C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_802206BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220728` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220804` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220940` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r3_80220AD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021DBE4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021E4F4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021E70C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021E7F4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021EAF0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021EB68` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021F308` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021F3CC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021F61C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021FBF8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_8021FF20` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r4_802204D0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r5_80220554` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r7_8021D8E8` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r7_8021F66C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss1_physics_r7_8021F6E4` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_8021DBE4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_8021DC18` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_8021ED5C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_8021ED90` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_80221E08` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_80221E38` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_80221ED8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_8022478C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_802247C0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_80224F98` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r16_80224FC8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021D974` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021DC7C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021DCF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021DEDC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021DF1C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021E150` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021E1F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021E4D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021E73C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021EAF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021EC60` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021EF98` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021F0D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021F384` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021F3C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021F694` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021F7F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021FAC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8021FD34` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220144` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220168` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220304` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220468` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220844` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022089C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802208B4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802208E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022094C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220A1C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80220E80` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80221090` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022114C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802218D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80221960` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80221A0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80221D44` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80221F94` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802233F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80223424` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80223468` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80223494` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80223550` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802236C8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802236E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802238B8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80223F24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802241DC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224204` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224384` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224680` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224AD8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224CB0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224DD4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224EC4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80224FF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80225024` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80225050` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022508C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802250AC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022520C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80225E10` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80225E50` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80225EC4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022614C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022616C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802261B4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802263E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_8022644C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80226AA4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80226F04` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802271BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227270` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227684` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_802276F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227860` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227ADC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227B40` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227B8C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80227F0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80228548` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80228594` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r2_80228998` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021DAE4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E028` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E2B4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E2D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E34C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E370` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E394` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E494` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E59C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E5C0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E634` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E658` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E67C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E710` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021E7D4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F0B0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F118` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F170` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F8C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F8E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F95C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F980` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021F9A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FA8C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FB94` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FBB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FC2C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FC50` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FC74` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FD08` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8021FDCC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80220668` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80220934` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80220AF0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80220CFC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80220F58` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221228` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221288` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802212A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802214D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221668` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221950` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8022199C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221BB0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221DC4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80221EF8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222120` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802221D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222408` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222AA0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222F48` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222FD0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80222FF0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8022300C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802237EC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802239A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80223DA4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80223DC8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80223E68` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80224050` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802241A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80224490` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80224E80` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80224F18` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80225A38` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80225DDC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802260E0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80226374` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802268EC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80226960` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80226B64` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227204` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227370` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227394` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802273B8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227518` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227790` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227D30` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80227F80` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80228084` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802280B8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802282A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_802282C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80228338` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_8022835C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80228380` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r3_80228900` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021D808` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021E260` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021E9B0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021EE94` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021F180` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8021F870` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80220030` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80220790` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80220A60` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802211C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8022181C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80221C50` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80221FE8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80222170` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80222604` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80222C88` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802230E8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80223850` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80223BB8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802240C0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80224268` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80224330` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802244E8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80224624` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80224B98` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225178` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225330` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802253E0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225498` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225554` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225594` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802256BC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802258EC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802259BC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225AD4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225BEC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225C9C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80225F80` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8022655C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80226628` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_8022673C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_802267F0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80226BB8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80226CB4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80226ECC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80227A84` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r4_80227F1C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r5_802224D0` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r5_80225B18` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r5_8022666C` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r5_80226AEC` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_actor_r6_802205E8` | `ctx->r6` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_8021DA00` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_8021EC24` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_802216B4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_802217C4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_80221D78` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_80224668` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r16_80224AD0` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r17_80221778` | `ctx->r17` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021D9DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021DBF8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021DE7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021E258` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021E4A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021EB48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021ECD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021ED70` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021EE8C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021EED8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021EF74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021F0A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021F168` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021F2B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021F4D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021F868` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021FAA0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8021FF90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220020` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022026C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220740` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220758` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802207CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220A68` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220BD0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220C74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220D4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80220FAC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802211BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802211F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221220` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221368` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802213DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221484` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022150C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221884` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221AC4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221B7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221BA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221C00` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221C30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221DBC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221E1C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80221E48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80222178` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802223A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802224DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80222698` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80222A74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80222E88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80222F60` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223024` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223148` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223318` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223334` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223370` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223394` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802233C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223694` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223780` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802237E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802238F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80223958` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224048` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022409C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022419C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802243E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224470` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802244B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224704` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802247A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224BA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224C40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224D28` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224E78` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224F10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224F7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80224FA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802252BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802253B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022550C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80225588` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80225820` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80225DCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80225F4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802260D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80226294` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80226490` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802265E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802268DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80226958` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80226B7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80226C24` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227128` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802271E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227228` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227510` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227628` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227798` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227C54` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227C88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227D28` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80227EBC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_8022807C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802280A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802280D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802284A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802284F4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_802288F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r2_80228B50` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021D718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021D9A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DA58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DA84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DB44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DBA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DC28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DDCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DE28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DEB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021DFF8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E0B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E1A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E22C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E284` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E3F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E448` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E534` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E56C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021E8D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EB20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EBD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EC04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021ECB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021ED1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EDA0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EE04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021EE60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F188` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F260` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F294` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F35C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F494` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F514` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F5AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F6F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F78C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F83C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021F894` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FA04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FA40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FB2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FB64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FECC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8021FF48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802200F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220360` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220564` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022071C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220868` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220A40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220B90` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220C9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80220DDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221170` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221348` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802213C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221418` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802214F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221680` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221760` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022180C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802218C0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221938` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221B1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80221C48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222150` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802222A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802222C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222328` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802225C0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222678` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802226DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222744` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222BD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222BF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222D40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80222F94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223048` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802230A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802230F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223110` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223290` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802233B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223440` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802234C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223684` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802236B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802237D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223848` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022388C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223970` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223A10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223B94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223CA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80223FF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224074` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802243B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022454C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224588` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802245B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802246DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022474C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802247D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224834` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224AA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224B84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224C74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224CA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224D74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224DAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80224E48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225098` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225130` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225268` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802252F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225350` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225440` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022554C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022563C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225738` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802257E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802258D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225930` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802259FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225A7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225ACC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225AFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225B74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225D20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225E08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80225EF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226258` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226454` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226508` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802265A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226620` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226650` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802266C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802267E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226AB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226B40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226CA0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226E50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226E78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226EC4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80226FCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022705C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802270A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227194` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227260` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802272D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_8022759C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802276D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227818` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802278D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802278F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227924` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227A14` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227A8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227BB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227C48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227CF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227D78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227E5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227F24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227F64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80227FB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228040` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228110` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228198` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228224` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228270` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802283E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228644` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228820` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_802289C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r3_80228C2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021D97C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021E030` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021E0F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021EAFC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021EEE4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021EF80` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021F0DC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021F2E0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021F5F4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8021F63C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80220470` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_802205B4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_8022084C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80220A24` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80221154` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80221F00` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80222128` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_802223E4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80222460` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80222F50` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80223750` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80223998` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80223E70` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80223F50` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_802245E0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80225750` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80225B10` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80226358` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80226534` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80226664` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80227A2C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80227BE0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80227F88` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80228444` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_802286C4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80228A4C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r4_80228B8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80221518` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_802219A4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80221A48` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80221CE0` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80222CA4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80224308` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80225998` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_80226C44` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r5_802275F4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r6_80224904` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r6_80225D3C` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r6_80226814` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss2_physics_r7_80220064` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r16_8021DEDC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r16_8021DFF8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r16_8021E2F8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021D914` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021D94C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021D9A8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021E59C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021E5E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021E9F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021EA1C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021EAF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021EB18` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021F0B8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021F4E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021F698` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021F870` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021F8FC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8021FD08` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220078` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_8022012C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220578` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220608` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_802207D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220824` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220A30` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220BEC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r2_80220DD4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021D724` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021DA78` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021DB6C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021DCDC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021DD70` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021DF00` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021E038` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021E38C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021ED50` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021EE0C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021EEA4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021EEDC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021EF40` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021F1E0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021F264` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021F2E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021F78C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_8021FF00` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_802200C8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_80220248` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_80220320` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_802204C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r3_80220778` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021D778` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021E144` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021E414` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021E62C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021EA98` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021F084` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021F3C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021F804` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_8021FCCC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_802203F4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_80220880` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_actor_r4_80220ACC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021E27C` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021EBAC` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021FAFC` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021FE98` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021FED0` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_8021FF48` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r16_80220124` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021D784` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021D8B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021D8F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021DA10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021DA48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021DCD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021DE64` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021E3DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021E3FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021E46C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021E654` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021E684` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021ED48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021EE34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021EF08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F1E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F25C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F350` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F538` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F5E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F960` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021F998` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021FCC4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021FD84` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021FEF8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8021FFF8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_802200C0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_802201F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220210` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220240` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_802203EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220680` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220700` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_8022071C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220738` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220758` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220770` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220878` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220A94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220C10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220C40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220CE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r2_80220D10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D73C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D754` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D834` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D858` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D890` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D8D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021D90C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021DB84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021DB9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021DC08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021DE38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021DF60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E124` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E254` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E2F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E3B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E538` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021E998` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021ED24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021EDA0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021EF60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F010` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F054` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F074` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F0B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F178` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F1C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F208` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F37C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F3BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F458` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F7D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F7FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F878` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F8C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F924` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021F9A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FAD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FB40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FBC4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FCF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FD2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_8021FFA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220034` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_802201C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220360` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_802203B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220658` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220990` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220A00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220A6C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220B98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220C60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220C84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220D90` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r3_80220E08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021DF1C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021E01C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021E2A4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021E340` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021EBEC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021EDB8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021F114` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021F2D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021F794` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8021FE58` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8022044C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_802205BC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_8022081C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_80220D20` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r4_80220EC8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r5_8021FCA4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r6_8022026C` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r7_8021EC1C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss3_physics_r7_80220AC4` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_8021D924` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_8021EBAC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_8021ED00` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_8021F198` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_802201A8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_802201E4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80220804` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80220840` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80220FA0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80220FDC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80221544` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r16_80221580` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r17_8021D8F4` | `ctx->r17` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021D7F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021D994` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021DE04` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021E3E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021E6AC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021E7C8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021E9BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021F010` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021F0FC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021F24C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021F4A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021F684` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021FBA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021FBC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021FC24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021FEF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_8021FFD4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_802205EC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220708` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220944` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220AD4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220AF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220B2C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80220D6C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_802213DC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_802219C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80221AC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80221B94` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80221F78` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_80222010` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r2_802223D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021DA60` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021E034` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021E180` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021E658` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021E788` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021ED14` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021EF0C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F2C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F37C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F3D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F414` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F484` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F844` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021F964` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021FA64` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021FC94` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8021FD50` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8022005C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_80220120` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8022076C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_80220CD4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_80220E8C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_80221EB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_80222048` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r3_8022214C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021D720` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021D754` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021D868` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021DAA0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021DC00` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021E234` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021E320` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021E354` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021EAAC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021EBC0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021F5B8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021F6D0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021FABC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021FDD0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_8021FE84` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_80220494` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_80221294` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_80221844` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_actor_r4_80221878` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r16_802200BC` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r16_802206A4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r16_80220E38` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r16_80221490` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DA68` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DA98` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DBF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DC3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DD60` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DD90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DE20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DEF4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021DF74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021E178` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021E1D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021E314` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021E780` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021EA9C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F2BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F3D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F568` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F580` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F5EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021F98C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021FAB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021FCF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021FD40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021FD7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_8021FE6C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220040` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220198` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220310` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_802207F4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_802209AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220CCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220F2C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80220F90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80221104` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80221164` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80221534` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_802216BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_802218E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80221D08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r2_80222050` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021D6E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021D718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021D7DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021D89C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021D974` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021DBB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021DD00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021DDCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E0B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E14C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E1A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E2F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E524` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E684` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021E928` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021EBC8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021EC3C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021EC94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021ED74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021EE58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021EFD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F024` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F0E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F1CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F3AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F544` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021F638` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021FA8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021FCD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021FDB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8021FE4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220088` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802201FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802202D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802202E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220504` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220598` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802205F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220668` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802206D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220858` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220930` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220994` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220C38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220D30` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220E00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80220FF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802210DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221384` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802213E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8022145C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221598` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221614` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221684` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221A74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221AD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221B44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221C70` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221F98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_80221FDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_8022208C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802222C0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r3_802224AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021DC84` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021DE58` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021E3A4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021E54C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021E9C4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021ED20` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021F384` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021FA6C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021FC80` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021FF7C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8021FFBC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_802200FC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8022031C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80220518` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80220754` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_802209B8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80220C4C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80220D74` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80220E68` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80221110` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_802211C0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80221304` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_802214C0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_802216C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80221758` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80221914` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_80221D14` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r4_8022211C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r5_8021EA48` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r5_8021F824` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r5_80220390` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r7_8021D77C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r7_8021FF14` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss4_physics_r7_80222324` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021E76C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021E86C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021E8D0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021E900` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021F1B0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021F498` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021F6EC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021FCC8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8021FEA0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_80220630` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_802206B8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_80220AEC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_80220D34` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_802210F8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_802211F8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8022125C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r16_8022128C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021D894` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021DA50` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021DB50` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021DDE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021E05C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021E09C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021E200` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021E47C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021E54C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021EA24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021EA70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021EB6C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021EBD8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021EBF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021F758` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021FB2C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021FE20` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021FEE0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021FF38` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8021FFA4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80220524` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8022071C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_8022079C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80220BC0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80220D58` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80220ED0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80221358` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r2_80221604` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021D720` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021DCC8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021E2F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021E664` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021E738` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021E978` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021ED14` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021F064` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021F7F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021FB1C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021FB70` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8021FBC4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_80220188` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_80220500` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_802206F4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_8022077C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_802208DC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_80220F88` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_802210C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r3_80221514` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021D8D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021DB8C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021DC80` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021E1C8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021E264` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021E818` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021EAD4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021EFDC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021F0B8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021F240` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021F328` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021F84C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021F91C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8021FBE8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_802201D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_802202AC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_80220980` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_802209C0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_802211A4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_80221398` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_802213C0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r4_8022169C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_actor_r5_80220010` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r16_8021E6D4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r16_802205C8` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r16_80221060` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021D7D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021D918` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021D980` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021D9F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021DC2C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021DD1C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E1C0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E388` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E4DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E4F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E66C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E730` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E764` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E864` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E8B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E8E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021E970` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021EB20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F070` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F13C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F288` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F374` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F594` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F5FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F7F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021F844` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021FB48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021FB68` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021FD34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021FDC4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8021FDE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802201CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220450` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220690` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802206C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8022075C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220898` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220918` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220B10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220B7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220C60` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80220F98` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802210BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802210F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802211F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80221240` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8022126C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802214EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_8022150C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80221574` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_802217D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r2_80221834` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021D77C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021DAFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021DB60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021DBBC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021DF44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E28C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E4B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E638` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E698` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E918` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021E9B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021EA38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021EA5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021EB74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021EF3C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021EF64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F02C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F088` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F0B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F274` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F418` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F538` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F578` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F650` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F710` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021F81C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021FA54` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021FBE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8021FD90` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220008` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220134` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_802201A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8022028C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_802205F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220810` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8022093C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220978` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_802209B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220B8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80220DB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_802212A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80221360` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_802213A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8022154C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80221594` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_8022160C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r3_80221694` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021E0A4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021E2F0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021ED1C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021EDE0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021F1D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021F4BC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021F620` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021F760` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021FB94` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021FCEC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021FEE8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_8021FFC4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_80220484` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_80220614` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_802209F0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_80220A88` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r4_80220EAC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r5_8021EE2C` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r5_8021EEAC` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r5_80220208` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r5_802204E4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r6_8021D950` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r6_8021F398` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r6_8021F9C4` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss5_physics_r7_8021E168` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r16_802217E4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r16_80221814` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r16_802218B4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r16_80224544` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r16_80224578` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021D94C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021DB0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021DB9C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021DC14` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021DE00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021DE40` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021E074` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021E11C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021E3F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021E660` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021E9F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021EC48` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021ECC0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021EE8C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021EECC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021F19C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021F300` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021F5D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021F83C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021FC4C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021FC70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021FE0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8021FF70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8022034C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802203A4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802203BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802203F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80220454` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80220524` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80220988` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80220B98` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80220C54` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80221544` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802215E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80221620` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80221668` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80221718` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80221978` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80222FA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80222FCC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223010` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8022303C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802230F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223270` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223288` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223460` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223ACC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223DF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223E1C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80223F9C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80224314` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80224394` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8022442C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80224758` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225434` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225474` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802254E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802257F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225818` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225860` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225A90` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80225AF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80226150` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8022660C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802268C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80226978` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80226D8C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80226E00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80226F68` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_802271E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80227248` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80227294` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80227614` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80227C4C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_80227C98` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r2_8022809C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021DF4C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E1D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E1FC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E270` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E294` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E2B8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E3B8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E4C0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E4E4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E558` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E57C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E5A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E634` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021E6F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021EBB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F3CC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F3F0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F464` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F488` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F4AC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F594` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F69C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F6C0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F734` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F758` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F77C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F810` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8021F8D4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220170` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8022043C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802205F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220804` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220A60` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220D30` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220D90` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80220DA8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802210FC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80221688` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80221798` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802218E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802219E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80221B38` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80221BE8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80221E20` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802224D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80222978` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80222B78` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80222B98` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80222BB4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223394` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223548` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8022394C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223970` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223A10` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223BF8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80223DBC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80224098` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80224FF0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80225400` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8022578C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80225A20` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80225F98` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8022600C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226210` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_8022690C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226A78` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226A9C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226AC0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226C20` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80226E98` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227438` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227624` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227684` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227788` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802277BC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802279A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_802279C8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227A3C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227A60` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80227A84` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r3_80228004` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8021D7E0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8021E184` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8021E8AC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8021F378` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8021FB38` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80220298` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80220568` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80220CCC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022148C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80221A20` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80221B88` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022201C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802226B8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80222A54` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80222C90` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802233F8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80223760` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80223CD8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80223E80` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80223F48` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224180` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802242B8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224350` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802245AC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022487C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022492C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802249E4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224AA0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224AE0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224CBC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224EA4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80224F74` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022508C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_802251A4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80225290` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022562C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80225C08` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80225CD4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80225DE8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80225E9C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80226264` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_80226360` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022657C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r4_8022718C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r5_802211A4` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r5_80221EE8` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r5_802250D0` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r5_80225D18` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r5_80226198` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_actor_r6_802200F0` | `ctx->r6` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r16_80221310` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r16_80221434` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r16_8022174C` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r16_802243B8` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r17_802213E8` | `ctx->r17` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021D9B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021DDA0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021E17C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021E3CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021EA58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021EDBC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021EFE0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021F370` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021F5A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021FA98` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021FB28` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8021FD74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220248` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220260` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802202D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220570` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802206D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8022077C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220854` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220AB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220CC4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220CF8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220D28` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80220E84` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802210E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221154` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221184` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221274` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802214F4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221790` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802217F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221824` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221B90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221DB8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80221EF4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802220B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802224A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802228B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222990` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222BCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222CF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222EC0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222EDC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222F18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222F3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80222F70` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8022323C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80223328` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8022338C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80223498` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80223500` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80223BF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80223DB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224004` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224078` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802240CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224558` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224588` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802246C0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224808` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224904` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224A58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224AD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80224DD8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802253F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802255F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80225784` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80225940` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80225B3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80225C90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80225F88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226004` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226228` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802262D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802265CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226838` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802268E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226930` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226C18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226D30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80226EA0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_8022735C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227390` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227430` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802275C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227780` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802277AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_802277DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227BA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227BF8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80227FFC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r2_80228254` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021D718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021D978` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DA2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DA58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DB44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DCEC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DD4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DDD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DF1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021DFDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E0C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E150` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E1A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E318` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E36C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E458` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E490` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021E7F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EA1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EAE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EB10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EBF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021ED98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EE64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021EF9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F01C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F0B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F1F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F294` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F344` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F39C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F50C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F548` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F634` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F66C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021F9D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021FA50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021FC00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8021FE68` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022006C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220224` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220370` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220548` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220698` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802207A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802208E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220C78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80220E50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022119C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022125C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802212DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802213D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022147C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221530` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802215A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221B68` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221CBC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221CDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221D40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80221FD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222090` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802220F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022215C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222600` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222624` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222770` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802229BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222B18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222B3C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222BF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222C4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222C98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222CB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222E38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222F58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80222FE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022306C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022322C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022325C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022337C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802233F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223434` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223518` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802235B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022373C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223850` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223B98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223C1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80223FD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802240FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022416C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802241E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224220` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022424C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802243E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224404` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224484` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224698` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802247B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224840` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022489C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022498C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224A98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224B50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224B98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224D44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224D9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224E88` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224EE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80224FB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225034` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225084` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802250B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022512C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225344` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022542C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225514` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225904` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225B00` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225BB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225C50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225CCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225CFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225D74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80225E8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022615C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802261EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022634C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802264FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226528` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226574` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802266D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226764` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802267B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022689C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226968` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802269DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226CA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226DD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226F20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226FD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80226FFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022702C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022711C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227194` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802272B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227350` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802273F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227480` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227564` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227668` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802276BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227744` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227814` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_8022789C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227928` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227974` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227AE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227D48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80227F24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_802280C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r3_80228330` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021D954` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021D9CC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021DF54` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021E01C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021E9F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021EA84` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021EDE8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021F0FC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021F144` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8021FF78` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802200BC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80220354` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8022052C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80220C5C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80220F24` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802218D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802219A8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80221B40` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80221DFC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80221E78` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80222980` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80222AF0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802232F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80223540` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80223A18` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80223AF8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80224274` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802242FC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80224D1C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802250C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80225A04` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80225BE0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80225D10` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80227134` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_802272E8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8022762C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_8022768C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80227B48` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80227DC8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80228150` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r4_80228290` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80220E98` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80221280` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_802216B4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_802226D4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80223F20` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_802245B4` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80224C10` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80224F50` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_802262F0` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r5_80226CFC` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r6_80225360` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r6_80225EC0` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r6_802267F4` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss6_physics_r7_8021FB6C` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r16_8021E23C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r16_8021E654` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021D910` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021DA24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021E31C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021E798` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021E948` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021E994` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021EAA4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021EBE0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r2_8021EF20` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021D720` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021D89C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021DEFC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021E364` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021E390` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021E3F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021E908` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021EB38` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r3_8021EF64` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021D8D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021E05C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021E2C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021E5E8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021EA68` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_actor_r4_8021EC14` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021D8CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021D9B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021D9D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021DB7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021DBB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021DD58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E270` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E380` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E414` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E440` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E4D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E534` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E5D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E698` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E7DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021E8C0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021ED7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021EE08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021EE38` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021EE8C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r2_8021EFA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021D990` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021DADC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021DFF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E16C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E198` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E1C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E220` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E2CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E574` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E618` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E64C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E700` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E7A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E84C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E8B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021E8E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EA8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EB84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EBE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021ECC0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021ED60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EE54` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EEB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r3_8021EF18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r4_8021D860` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r4_8021DF04` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r4_8021E3C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r4_8021EB30` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r5_8021E10C` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r5_8021EC0C` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `boss7_physics_r7_8021E008` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021DC30` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021E06C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021E6B4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021EBCC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021EF4C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021F354` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_8021FA38` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_802200B4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r16_80220414` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021DB24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021DB5C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021DEF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E0A4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E3C8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E420` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E450` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E474` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E4CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E5C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021E6E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EAA4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EABC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EB10` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EC04` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EC78` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021ED90` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021EDF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021F1C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021F38C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021F87C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021F8E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021FCB8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021FDA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021FDC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021FE24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_8021FFC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_802200F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_80220450` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r2_80220508` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021DA5C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021DC6C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021DD30` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021E394` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021E960` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021EF84` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021F748` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_8021FA70` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_802202E0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_802204E4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_802205D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r3_802206C0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021D9EC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021E010` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021E4E4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021E794` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021E9C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021F2CC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_8021F52C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_actor_r4_802201D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r16_8021FE64` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021D9DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021DCE0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021DE00` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021DF9C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E000` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E0E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E2E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E4F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E688` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E708` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E75C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021E9B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021ECCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021EFFC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F0B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F258` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F2BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F3C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F44C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F4AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F760` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021F7B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021FAE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021FBAC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021FC34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021FCE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_8021FEA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_80220154` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_802202FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_80220358` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_802205D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_802206D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r2_80220760` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021D9B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DA90` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DAE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DCB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DD64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DE54` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DECC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021DF60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E034` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E198` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E23C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E334` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E5E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E72C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E98C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021E9FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EB44` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EBB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EC20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EC64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EC9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021ECFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021ED50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EED0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EF38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021EFD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F014` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F130` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F324` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F430` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F47C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F770` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F788` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F7E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F83C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021F9BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021FA24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021FAC0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021FC8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8021FD38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220004` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220020` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220098` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220124` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8022030C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220324` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_802203E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220484` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_802205AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_80220740` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r3_8022077C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021DB40` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021DC88` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021DEFC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021E264` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021E344` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021E968` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021EAC4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021EDAC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021EFA0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021F1C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021F750` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021F898` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021FA8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021FF48` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_8021FF84` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_802202E8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_80220398` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_802204CC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r4_80220614` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r5_8021DA24` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r5_8021DCF8` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `brobodc08_physics_r6_8021FB10` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `candidate_contact_physics_r4` | `ctx->r4` | Candidate physics after the actor argument is replaced by its physics pointer | 1 |
| `cat_contact_mode_r3` | `ctx->r3` | Shared contact mode before the checked sound command | 1 |
| `character_status_r3` | `ctx->r3` | Байтовая запись здоровья текущего персонажа из8021D4A8 | 1 |
| `character_status_r4` | `ctx->r4` | Байтовая запись здоровья текущего персонажа из8021D4A8 | 1 |
| `churattobanyatto_actor_r16_8022DB58` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022DCBC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022DD20` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022DDDC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022E0D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022E2F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022E4B8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r2_8022E668` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022D790` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022D8F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022DA2C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022DAD0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022DB48` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022DFB0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r3_8022E984` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r4_8022D808` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r4_8022D924` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r4_8022DE24` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r4_8022E870` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_actor_r4_8022EA30` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r16_8022E2B4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r16_8022E570` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r16_8022E5D4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022D7A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DA24` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DA4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DAC8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DB40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DBD8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DD6C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022DFC8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E05C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E148` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E194` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E210` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E270` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E358` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E3C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E454` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E488` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E510` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E6DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E728` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E7A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E808` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E83C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E868` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E8D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r2_8022E9E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022D7B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022D7D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022D7FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022D898` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022D8C0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022DBB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022DCC4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022DD4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022DFD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022DFF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E0A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E30C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E440` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E4E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E5AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E630` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E930` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r3_8022E9B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022D798` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022D834` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022DD28` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022DFB8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022E040` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022E4C0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022E650` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022E944` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r4_8022E98C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r7_8022D934` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `churattobanyatto_physics_r7_8022DA64` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `city_wait_count_r3` | `ctx->r3` | Private signed wait counter after its lhu read, increment/store and sign extension; before v1 becomes actor pointer | 8 |
| `constructed_common_actor_r17` | `ctx->r17` | Actor выбранного common slot: его новый physics уже опубликован, далее передаётся parser и сбросу transform | 1 |
| `constructed_common_physics_r16` | `ctx->r16` | Новый allocation F4 именно этой итерации, сохранённый в actor98 перед вызовом parser | 1 |
| `constructed_parent_actor_r16` | `ctx->r16` | New parent actor, possibly null, retained through sprite/child initialization | 1 |
| `contact_candidate_actor_r16` | `ctx->r16` | Advancing world actor cursor before projectile/pair loops reuse the register | 1 |
| `contact_candidate_pool_index_r17` | `ctx->r17` | Pool index30..229 during the player contact search | 1 |
| `contact_first_actor_r21` | `ctx->r21` | First actor retained through geometry, reciprocal slots and contact angles | 1 |
| `contact_mode_r16` | `ctx->r16` | Режим контакта8021BCE4, сохраняется в s0 через E70E0 | 1 |
| `contact_mode_r3` | `ctx->r3` | Contact mode loaded from the shared event global before dispatch | 1 |
| `contact_mode_r3` | `ctx->r3` | Режим контакта, загруженный из8021BCE4 до выбора ветки | 1 |
| `contact_parent_actor_r5` | `ctx->r5` | Parent actor whose dialogue guard and state write precede the game-flag call | 1 |
| `contact_player_actor_r18` | `ctx->r18` | Player actor during the player-versus-world prefix only | 1 |
| `contact_player_physics_r5` | `ctx->r5` | Player physics read for cooldown and existing contact slot | 1 |
| `contact_second_actor_r20` | `ctx->r20` | Second actor retained through reciprocal contact publication | 1 |
| `controller_index_r16` | `ctx->r16` | Индекс контроллера; не указатель actor. Значение входного a0 сохранено в s0. | 1 |
| `counter_bank3678_delta_r5` | `ctx->r5` | Exact Original register storage in this proven instruction region only | 1 |
| `counter_bank3678_index_r4` | `ctx->r4` | Exact Original register storage in this proven instruction region only | 1 |
| `created_minicar_r18` | `ctx->r18` | Результат E6968, живой actor MiniCar до восстановления s2 | 1 |
| `created_npc_actor_r16` | `ctx->r16` | Allocated actor returned by descriptor construction, including the checked null-allocation path | 8 |
| `created_npc_physics_r18` | `ctx->r18` | Physics record of the successfully created actor | 8 |
| `current_actor_a0_801E5ACC` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8022DA20` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8022E130` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8022EA10` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8022FA18` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_802305F4` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_80230DC4` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_80231B70` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_80231D6C` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_802323CC` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_80232840` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8023D848` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8023D89C` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8023E190` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8023E5E4` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a0_8023E7C8` | `ctx->r4` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_801EA59C` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_801EAACC` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_8022FBD4` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_8023DAB0` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_8023DE60` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_a1_8023EFFC` | `ctx->r5` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_801EAE20` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8022F1E4` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_80231058` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023109C` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_80231140` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023D754` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023D998` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023DD04` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023E150` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023E2B4` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023E860` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023EC6C` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023F7A8` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_8023F868` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s0_80240274` | `ctx->r16` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_s1_8023F854` | `ctx->r17` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801E590C` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801E94AC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801E96E8` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801EA0AC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801EA144` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_801EACC0` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022DAD8` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022E154` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022E210` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022E82C` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022ED48` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022ED78` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022EEE8` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F220` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F340` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F650` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F75C` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F824` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022F900` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8022FD08` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80230630` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80230660` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80230BC0` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80231BAC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80231BF0` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80231D90` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80232150` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80232458` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_80232864` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023D710` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023D7F4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023D894` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023D9AC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023DBE4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023DE28` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023DF18` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023DFCC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023DFF4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023E0D4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023E378` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023E39C` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023E9E0` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EA14` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EC10` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023ECEC` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023ED84` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EDA4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EE10` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EEA8` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023EF64` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023F650` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023F8D4` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v0_8023FC54` | `ctx->r2` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_801E92D0` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_801E99E0` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_801EA2A0` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8022EDC8` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8022F848` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8022F8C0` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023000C` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_80230C84` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_80230FC0` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023128C` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_80232330` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023D9FC` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023DE00` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023DEDC` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023DF3C` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023DFB4` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023E174` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023E354` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023E544` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023E798` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023E834` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023EC34` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023ECAC` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023F628` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_actor_v1_8023FCD8` | `ctx->r3` | ROM global current actor pointer, only until the register is assigned/clobbered again | 1 |
| `current_heading_f12` | `ctx->f12` | Входной current angle до перезаписи f12 целевым углом | 1 |
| `current_physics_a0_801E92C8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_801E99E8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_801EA058` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_801EA2A8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022DAE0` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022E218` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022EEF0` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022F764` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022F7F4` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022F890` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022FA40` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8022FBE8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_80230668` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_80230FB4` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_80231BF8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_80232460` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023D828` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023DA04` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023DD5C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023DEE4` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023DEF8` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023DF84` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023E17C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023E54C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023E83C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023E9B4` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023EBDC` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023EC7C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023EF7C` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a0_8023F7CC` | `ctx->r4` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a1_8022ED04` | `ctx->r5` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a1_8022EDEC` | `ctx->r5` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a1_8022EE98` | `ctx->r5` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a1_8023E4F4` | `ctx->r5` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a3_8022ECC0` | `ctx->r7` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_a3_8023F710` | `ctx->r7` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_s0_8023E05C` | `ctx->r16` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_s0_8023E734` | `ctx->r16` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801E592C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801E99FC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EA03C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EA0E0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EA2B8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EA594` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EAAC4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EADB0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EADBC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EAE18` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_801EB4D4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022DA54` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022E16C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022E7C4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022E860` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022E9F8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EC3C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EC70` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EC98` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022ED8C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EDA0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EDBC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EE28` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EE68` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022EEC4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F66C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F6A8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F6D8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F718` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F738` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F8C0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F940` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022F970` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022FA10` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022FA64` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8022FD3C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230030` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023006C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_802300B0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230610` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230BF4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230CA8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230CE0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230D44` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230DA4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230DAC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230DC8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80230FA4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231048` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231138` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231B8C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231E28` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231E58` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231E88` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80231EA8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80232184` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_80232344` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D724` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D7E4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D894` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D8BC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D90C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D954` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023D960` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 2 |
| `current_physics_v0_8023D96C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DCF4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DD30` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DD9C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DE74` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DEE8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023DF50` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E034` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E040` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E064` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E094` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E0B0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E0DC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E0F8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 2 |
| `current_physics_v0_8023E148` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E164` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E27C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E288` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E4C4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E4E4` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E518` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E594` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E728` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E7AC` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E808` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E990` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023E9B8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023ED2C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023ED5C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023ED68` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023ED74` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023EDF8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023EFC0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F068` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F198` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F1C0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F28C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F298` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F380` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F6E8` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F708` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F7A0` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F84C` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v0_8023F994` | `ctx->r2` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E52D8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E58E4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E5904` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E59A0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E5A98` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E5C30` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E92E8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E94C8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801E9704` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801EA004` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801EA118` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801EAD78` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_801EAE70` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022E840` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022E9AC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022EC1C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022F330` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022F9F0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8022FA8C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_80230134` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_80230D60` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_80230F4C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_802310EC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D6E4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D708` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D778` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D7D4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D804` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D848` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D8FC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D934` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D93C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D990` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023D9C4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DAC4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DCD0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DD64` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DDC0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DE60` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DE88` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DEC8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DF04` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DF0C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DF64` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DFBC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023DFF8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E0BC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E0CC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E0DC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E188` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E19C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E258` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E2AC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E2D8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E38C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E43C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E4C4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E5CC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E704` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E7D0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E970` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023E9E4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EA04` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EA18` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EA50` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EA6C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EAC4` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EB1C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023ED40` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EE00` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EF20` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023EF6C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F26C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F2E8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F3AC` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F6B0` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F8B8` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8023F91C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_80240238` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `current_physics_v1_8024026C` | `ctx->r3` | ROM global current physics pointer, only until the register is assigned/clobbered again | 1 |
| `derorin_actor_r16_8022FF44` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r16_8022FFAC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r16_8022FFE4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022ECA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022ED30` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022EEC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022EFB4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022F26C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022F59C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022F6B0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022F7D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022F860` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022FA70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022FB88` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8022FCD8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_8023010C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_80230314` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r2_80230464` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022EC60` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022ED5C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022EDE4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022F128` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022F170` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022F790` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022F88C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022FE30` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8022FF0C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_80230024` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r3_8023023C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022EE20` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022F3BC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022F540` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022F668` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022FB10` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8022FC80` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_802300D8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_802301FC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_actor_r4_8023040C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r16_8022EF98` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r16_8022F4FC` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r16_8022FA54` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8021F6B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8021F760` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8021F7A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022EC78` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022EE5C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022EE7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022EF18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022EF3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F060` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F098` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F2B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F344` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F444` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F520` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F7A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F988` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F9A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022F9F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022FAD0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022FB04` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022FE48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022FF54` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_8022FFF4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_802300D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_802301BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_802301F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r2_80230290` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8021F680` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022ED84` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022EEA0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022EF08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022EFAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F1B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F230` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F258` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F290` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F390` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F3C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F464` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F488` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F8BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022F8EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022FA68` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022FB64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022FE58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_8022FED8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_80230140` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_80230164` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r3_80230260` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8022EC68` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8022F798` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8022FB48` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8022FE38` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8022FE70` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_8023002C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_80230114` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r4_80230234` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r6_8022EC88` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `derorin_physics_r6_8022F7B8` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `descriptor_actor_r19` | `ctx->r19` | Actor whose resource groups and physics kind are initialized | 1 |
| `descriptor_cursor_r17` | `ctx->r17` | Advancing descriptor stream cursor, returned after FFF5 | 1 |
| `dialogue_descriptor_r4` | `ctx->r4` | Input descriptor until a0 is reused for runtime parameters | 1 |
| `dialogue_entry_physics_r3` | `ctx->r3` | Physics record used only for the initial dialogue latch read and write; v1 changes role afterward | 8 |
| `dialogue_state2_r16` | `ctx->r16` | State2 constant preserved across the dialogue call | 1 |
| `dialogue_state_r16` | `ctx->r16` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `doragonrock_actor_r16_8022E64C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r16_8022EDCC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r16_8022F19C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022D83C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022D900` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022DD88` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E03C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E0F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E40C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E4E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E508` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E690` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022E700` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022EA18` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022EA90` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022EC98` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022ED44` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022EE0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022F044` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022F1E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022F564` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r2_8022F61C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r3_8022DD08` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r3_8022DF34` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r3_8022F068` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r3_8022F3E0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r4_8022DD2C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r4_8022E454` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r4_8022E7F0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_actor_r4_8022F470` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022D888` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022D990` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022D9D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022DC5C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022DD00` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022DF4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E088` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E188` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E1C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E3D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E598` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E634` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E7E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E808` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022E980` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022EB90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022EBCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022EDC0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022EE78` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F180` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F23C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F3FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F480` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F4C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r2_8022F648` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022D774` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022D860` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022D8F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DA08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DA98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DB48` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DBE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DD34` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DDA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DF5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022DF74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E060` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E0E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E200` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E290` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E340` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E45C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E4A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E4F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E588` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E624` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E6C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E6F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E864` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E8B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E91C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022E9EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022EB80` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022EC20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022EE4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022EEEC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022EFEC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F014` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F218` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F2AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F2D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F40C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F460` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r3_8022F55C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022D844` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022DA18` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022DF3C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022E044` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022E210` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022E940` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022ECF8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022F0A0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022F130` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022F3E8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022F50C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r4_8022F624` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doragonrock_physics_r7_8022EE94` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; только до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r16_8022096C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r16_80220F90` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r16_8022F034` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r16_8022F38C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_802209D8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_80220B28` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_80220B64` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_80220E8C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_80220FC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r2_8022F3C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r3_802208CC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r3_80220938` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r3_80220F64` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r3_8022F0DC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r3_8022F1CC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r4_802209A8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r4_80221064` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_actor_r4_8022F06C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r16_8022F240` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_802208E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80220930` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80220A40` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80220B04` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80220BA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80220F5C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_80221024` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_8022EF88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_8022F150` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r2_8022F21C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_802208F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022090C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220A14` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220AB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220C30` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220CB0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220D58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220D8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220E2C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_80220FFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022106C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022EF60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022F128` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022F1EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022F310` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r3_8022F378` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_802208D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_80220D3C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_80220E40` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_80220EC0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_8022F0E4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `doridoritolneder_physics_r4_8022F1D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `drive_command_r4` | `ctx->r4` | Входная команда scalar drive; сохранена до конца функции. | 1 |
| `enemy_actor_r16` | `ctx->r16` | Указатель actor из global8021BDEC до следующей записи регистра | 20 |
| `enemy_actor_r16` | `ctx->r16` | Указатель текущего actor из global8021BDEC; scope заканчивается перед следующей записью этого регистра | 12 |
| `enemy_actor_r17` | `ctx->r17` | Указатель actor из global8021BDEC до следующей записи регистра | 1 |
| `enemy_actor_r17` | `ctx->r17` | Указатель текущего actor из global8021BDEC; scope заканчивается перед следующей записью этого регистра | 2 |
| `enemy_contact_mode_r3` | `ctx->r3` | Значение contact mode из global8021BCE4 до переиспользования v1 как указателя actor/physics | 10 |
| `enemy_shared_actor_arg_8002A3B0` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A3D4` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A3F8` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A454` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A47C` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A4A4` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A4F0` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A580` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A5A4` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A5F0` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_8002A628` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8A28` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8A34` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8A4C` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8A88` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8AA0` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_shared_actor_arg_801E8AAC` | `ctx->r4` | Входной ABI аргумент actor; только до первого изменения a0/вызова, ctx storage сохраняется | 1 |
| `enemy_state_r3` | `ctx->r3` | Actor94 state до первого private callback; r3 после jal может иметь другую роль | 10 |
| `enemy_state_r3` | `ctx->r3` | Actor94 state до первого private callback; после jal регистр меняет роль | 16 |
| `first_shape_index_output_r16` | `ctx->r16` | Output pointer for the first colliding shape index | 1 |
| `fisher_actor_r2_8021DA0C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r2_8021DBFC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r2_8021DCA8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r2_8021DE3C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r2_8021E534` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r3_8021D7D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r3_8021E2B8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r3_8021E3F0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021DB54` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021DC5C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E080` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E0D0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E1C8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E460` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E82C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E860` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_actor_r4_8021E91C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r16_8021E670` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r17_8021E628` | `ctx->r17` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021D7E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DA84` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DAEC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DB0C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DB4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DC4C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DEB0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DEFC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021DFA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E078` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E13C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E18C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E408` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E4E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E5EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E774` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E7EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E824` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r2_8021E8E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D7F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D810` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D834` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D87C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D8A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D8E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D918` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021D9C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021DC28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021DD30` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021DE20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021DF64` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E0DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E164` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E418` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E430` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E450` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E488` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E55C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E58C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E5AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E614` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r3_8021E8B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021D7D8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021D9D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021D9F4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021DC04` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021E028` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021E3F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `fisher_physics_r4_8021E53C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `flash_delay_count_physics_r3` | `ctx->r3` | Physics для чтения/записи count34; следующий instruction заменяет этот регистр signed selector | 1 |
| `flash_delay_initial_physics_r3` | `ctx->r3` | Physics текущего эффекта: чтение и установка однократного init latch28 | 1 |
| `flash_motion_initial_physics_r3` | `ctx->r3` | Physics эффекта до вызова вертикального impulse: selector44, opacity58 и radius5C | 1 |
| `flash_signed_delay_r3` | `ctx->r3` | Signed selector44; количество задержки равно его модулю, отрицательные варианты считаются отдельной ветвью | 1 |
| `flash_transition_actor_r3` | `ctx->r3` | Actor эффекта при публикации uniform scale1 и state10; здесь регистр уже не physics и не selector | 1 |
| `game_counter_delta_r5` | `ctx->r5` | Signed input delta consumed by the original byte addition | 1 |
| `game_counter_index_r4` | `ctx->r4` | Input counter index retained without register reuse | 2 |
| `gokireon_actor_r16_80221B04` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r16_80221D74` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r16_80222154` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r16_80222378` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r16_80222824` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r16_802229BC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80222868` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80222A14` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80222BB4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80222D28` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80222DF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80223308` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80223574` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_802235B0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r2_80223658` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_802217A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_80221A44` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_80221A78` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_80222340` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_802227E0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_802228C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_80222A48` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_80223398` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r3_8022351C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80221874` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80221CE8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80221F90` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_802220A8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80222128` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80222408` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_802225A0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_802225D0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80222BFC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80223144` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80223268` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r4_80223598` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r5_802217C8` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_actor_r7_80222C78` | `ctx->r7` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r16_80222634` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802218A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221950` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802219F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221B84` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221BE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221DFC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221E90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80221FAC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802221D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222348` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802223EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_8022258C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_8022272C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802228B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802229B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222A58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222C20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222DA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222EE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80222FCC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80223078` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802230CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80223180` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802231F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80223384` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802233CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802233F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_80223460` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r2_802234AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221800` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221844` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221868` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802218FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221980` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221AF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221B5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221BA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221BC8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221C24` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221C78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221CE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221DCC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221ED0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80221F10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222000` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222048` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802220B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222130` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802221AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802222C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222314` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802223CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222598` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802225C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022260C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022269C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802226BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802226DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222784` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222860` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222890` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802228DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222998` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222A80` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222AB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222B7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222BE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80222E70` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022301C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80223120` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022313C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802231C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022321C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80223248` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802232F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_8022342C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80223484` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80223590` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802235B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802235D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_802235F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r3_80223694` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_802217B8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_802219B0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80221A2C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80221A70` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80221E38` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80221F40` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80222214` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_802227D8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80222974` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80222A30` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80222B44` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80222F8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80223040` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80223390` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r4_80223560` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r5_80221CAC` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `gokireon_physics_r6_80222CE4` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `health_delta_r4` | `ctx->r4` | Входная прибавка к здоровью; r4 далее переиспользуется как указатель status. | 1 |
| `houdaimajinposedokirafujyaicong_actor_r16_80230D58` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r16_80230E40` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r16_8023182C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r16_80231860` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_8022EA00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_8022EB24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_8022EF24` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80230424` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_802304B4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80230810` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_8023083C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80230A3C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80230B4C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80230D6C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_80231628` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_802317A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r2_802318D8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_8022E930` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_8022EDE0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_8022F010` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_8023026C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_802303EC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_8023064C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_80230764` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_80230E54` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_80231574` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r3_80240400` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r4_8022EB64` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r4_802306C8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r4_802309A4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_actor_r4_802315D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8022E958` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8022EA70` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8022EBE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8022EE44` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8022EFF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80230284` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80230310` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_802303E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80230668` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_802306B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_8023075C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_802315C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80231658` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_802316EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80231740` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80231840` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80231870` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r2_80240418` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022E978` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EA28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EAF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EBAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022ED50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EE0C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EE78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EF08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EF40` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8022EF98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230294` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_802302AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230328` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8023036C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230678` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230690` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230980` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230A60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230C7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230D74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80230DD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_8023158C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_802315A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80231608` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80231688` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_802316FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r3_80240428` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_8022E938` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_8022EA08` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_8022EC08` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_8022EDE8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230274` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230654` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230818` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230A44` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230CC8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80230E5C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r4_80240408` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r6_8023070C` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `houdaimajinposedokirafujyaicong_physics_r7_8022EA80` | `ctx->r7` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r16_8022D7F0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r16_80230678` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r2_8022D8DC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r2_8022D958` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r2_8023F320` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r3_8022D78C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r3_8022D838` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r4_8022D7C4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r4_8022D9CC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r4_80230628` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_actor_r5_8023F3A8` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D790` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D7AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D7CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D7E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D894` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D914` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022D930` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022FB94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022FC1C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022FC88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8022FCD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8023F2F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8023F370` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8023F4C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r2_8023F5AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D75C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D7A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D7BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D850` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D86C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 2 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D8E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D908` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022D980` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022FABC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022FBF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022FDE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8022FE3C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023055C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_80230590` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_80230620` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F1D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F1F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F26C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F2A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F344` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F390` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F538` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r3_8023F574` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8022D840` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8022D960` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8022FFD8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_802305D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_802305F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8023F220` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8023F328` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r4_8023F3A0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `igaheadtutarubutterflysakanathunder_physics_r5_8022FF50` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `interaction_player_state_r3` | `ctx->r3` | Player state retained across the Original interaction state gates | 1 |
| `kaen_actor_r2_8022FEB4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_8022FEDC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_8023032C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_802306C8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_802306F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_8023096C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_80230DCC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r2_80230DF4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_802300BC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_8023044C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_80230468` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_8023060C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_80230950` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_80230C54` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r3_80230D74` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r4_802302A4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r4_80230658` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r4_80230AC4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r4_80230BE0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r4_80230CA4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_actor_r5_80230EA0` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_8022FF38` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230264` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230280` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_8023029C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_802303D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230460` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_8023048C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_802307F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230C5C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230E50` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r2_80230F6C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_8022FDDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_8022FE04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_8022FFB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_802300D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_80230144` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_802302F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_80230628` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_80230650` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_802307D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_802309C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_80230BE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r3_80230EA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_8022FE30` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_8022FE50` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_8022FE70` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_8022FFD0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_80230100` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_80230394` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_80230680` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_802306A0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_80230B94` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r4_80230D88` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r5_80230B44` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r5_80230BB8` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r5_80230C8C` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `kaen_physics_r6_802309E0` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `linked_cached_center_physics` | `ctx->r4` | Физика эффекта для parent7C и однократного сохранения XYZ родителя в float58/5C/60. | 1 |
| `linked_delay_count` | `ctx->r3` | Count34 после lhu, увеличения и приведения к signed16 для сравнения с delay36; последующий boolean не переименовывается. | 1 |
| `linked_delay_limit` | `ctx->r2` | Signed delay36 для сравнения с обновлённым signed count34. | 1 |
| `linked_initial_count_physics` | `ctx->r2` | Физика эффекта до вызова RNG; delay-slot store сбрасывает count34 в0. | 1 |
| `linked_initial_delay_physics` | `ctx->r3` | Физика эффекта для сохранения первого RNG delay20..49 в halfword36 до вызова markerCA setter. | 1 |
| `linked_initial_parent_actor` | `ctx->r2` | Адрес настоящего parent actor после прибавления pool base; читаются float XYZ9C/A0/A4. | 1 |
| `linked_initial_parent_pool_index` | `ctx->r3` | Signed halfword7C — индекс parent в общем actor pool; область заканчивается до превращения регистра в byte offset. | 1 |
| `linked_initialization_physics` | `ctx->r3` | Физика эффекта из8021BCDC при чтении и установке initialization latch28. | 1 |
| `linked_offset_x_physics` | `ctx->r3` | Физика эффекта при чтении сохранённого центра X из float58 для мгновенного RNG offset. | 1 |
| `linked_offset_y_physics` | `ctx->r3` | Физика эффекта при чтении сохранённого центра Y из float5C для мгновенного RNG offset. | 1 |
| `linked_offset_z_physics` | `ctx->r3` | Физика эффекта при чтении сохранённого центра Z из float60 для мгновенного RNG offset. | 1 |
| `linked_parent_physics_r2` | `ctx->r2` | Parent physics used for the pending clip read | 1 |
| `linked_parent_pool_index_r3` | `ctx->r3` | Parent pool index before address-scale arithmetic reuses this register | 1 |
| `linked_renewed_delay_physics` | `ctx->r3` | Физика эффекта для нового RNG delay36 и обнуления count34 перед clip0 command. | 1 |
| `linked_retirement_parent_index` | `ctx->r3` | Signed parent7C в финальном сравнении с-1; снятая связь вызывает настоящий Source destructor801E6E6C. | 1 |
| `linked_retirement_physics` | `ctx->r2` | Физика эффекта для финальной проверки parent7C; область заканчивается до записи constant-1 в тот же регистр. | 1 |
| `linked_teleport_x_actor` | `ctx->r2` | Настоящий actor эффекта для Source teleport X9C в delay slot следующего RNG вызова. | 1 |
| `linked_teleport_y_actor` | `ctx->r2` | Настоящий actor эффекта для Source teleport YA0 в delay slot следующего RNG вызова. | 1 |
| `linked_teleport_z_actor` | `ctx->r2` | Настоящий actor эффекта для Source teleport ZA4 после последнего RNG вызова. | 1 |
| `matrix_left_row_cursor_r4` | `ctx->r4` | Курсор строки левой матрицы; после обхода больше не используется как входной указатель | 1 |
| `matrix_product_output_r6` | `ctx->r6` | Выходная матрица, включая прямую запись последнего элемента и копирование остальных | 1 |
| `matrix_right_input_r5` | `ctx->r5` | Начало правой матрицы; до загрузки временного результата в этот регистр | 1 |
| `metabogabugon_actor_r16_8022DEB4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022DAEC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022DE30` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022DEF8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022DF7C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022E388` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022E3BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022E3F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022E728` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r2_8022E780` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022DA88` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022DE84` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022DFCC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022E2CC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022E490` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022E5E4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r3_8022E64C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r4_8022D8E4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_actor_r4_8022DB48` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r16_8022D820` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r16_8022D8BC` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r16_8022D9E4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022D7AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022D8D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022DA80` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022DE08` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022DF50` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022DFE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022E05C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022E488` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022E5DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r2_8022E6A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022D774` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DA58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DABC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DD94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DDDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DE38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DEF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022DF1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022E08C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022E4B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022E5C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022E688` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r3_8022E708` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r4_8022DE48` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r4_8022E234` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `metabogabugon_physics_r5_8022E2AC` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `minicar_physics_r17` | `ctx->r17` | Physics98 созданного MiniCar, route settings/counters | 1 |
| `minicar_route_points_r17` | `ctx->r17` | Таблица signed-short X/Z waypoint pairs из physics48 | 1 |
| `model_opacity_f2` | `ctx->f2` | Float opacity loaded from a1 bits, retained through the exact1 comparison and final store | 1 |
| `npc_contact_mode_r3` | `ctx->r3` | Contact mode through the complete mode switch, before v1 is reused for state guard arithmetic | 8 |
| `npc_entry_physics_r3` | `ctx->r3` | Physics record only across the initial latch28 read/write; v1 is reused afterward | 10 |
| `object_actor_r4` | `ctx->r4` | Actor, из которого читаются X/Z перед вычислением направления на игрока | 1 |
| `object_physics_r16` | `ctx->r16` | Physics record для speed/heading и готовых X/Z velocity | 1 |
| `object_physics_r16` | `ctx->r16` | Physics record для speed/pitch и готовой Y velocity | 1 |
| `object_physics_r3` | `ctx->r3` | Physics pointer для флага однократной остановки | 1 |
| `object_physics_r3` | `ctx->r3` | Physics pointer до проверки/записи входа state2 | 1 |
| `object_physics_r3` | `ctx->r3` | Physics pointer до проверки/записи однократной инициализации | 1 |
| `object_physics_r3` | `ctx->r3` | Physics pointer до установки входного флага state2 | 1 |
| `object_physics_r3` | `ctx->r3` | Текущий physics record до однократного вызова выбора клипа | 1 |
| `opacity_model_r4` | `ctx->r4` | Unchanged model pointer for opacity flag/time writes | 1 |
| `oppi_actor_r16_8023021C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r2_8022F86C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r2_8022FDE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r2_80230258` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022F680` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022F6BC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022F8B4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022F900` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022F938` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_8022FE58` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r3_802300C8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r4_8022F9E4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r4_8022FA18` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r4_8022FA4C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r4_8022FB20` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_actor_r4_8022FC88` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022F740` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022F8AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022F978` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FBD8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FC54` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FCD8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FD44` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FD68` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_8022FEA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_802300E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_80230134` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_80230158` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r2_802302B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F6D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F718` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F79C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F7D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F864` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022F950` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FAAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FAF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FB18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FC80` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FD1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FDB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_8022FF60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_802300F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_80230108` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_802301C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_80230274` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r3_80230298` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_8022F688` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_8022F8F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_8022FB78` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_8022FE50` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_8022FF04` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_802300D0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `oppi_physics_r4_80230168` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `panel_record_r7` | `ctx->r7` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `panel_update_slot_r16` | `ctx->r16` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `participant_actor_r19` | `ctx->r19` | Actor для публикации трёх angles до восстановления s3. | 1 |
| `participant_control_r18` | `ctx->r18` | Сохранённый participant control до восстановления s2 в epilogue. | 1 |
| `participant_pool_control_r16` | `ctx->r16` | Текущая60-entry control record в loop body. | 1 |
| `periodic_initial_physics_r3` | `ctx->r3` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `periodic_random_x_actor_r3` | `ctx->r3` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `periodic_scale_y_actor_r3` | `ctx->r3` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `periodic_threshold_actor_r4` | `ctx->r4` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `periodic_velocity_physics_r4` | `ctx->r4` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `pickup_attached_parent_actor_r16` | `ctx->r16` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_attachment_slot_r16` | `ctx->r16` | Attachment-slot loop0..2 only; register reused for the effect index after this scope | 1 |
| `pickup_collect_initial_physics_r3` | `ctx->r3` | Physics pointer only for the initial latch28 read/write before register reuse | 1 |
| `pickup_dispatch_state_index_r3` | `ctx->r3` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_dispatch_state_r2` | `ctx->r2` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_effect_index_r16` | `ctx->r16` | Six-effect creation loop index0..5 written into spawned physics44 | 1 |
| `pickup_expand_scale_actor_r16` | `ctx->r16` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_first_arc_actor_r16` | `ctx->r16` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_first_arc_z_actor_r17` | `ctx->r17` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_flash_cosine_actor_r16` | `ctx->r16` | Exact Original register storage for this proven instruction region only | 1 |
| `pickup_flash_sine_actor_r16` | `ctx->r16` | Exact Original register storage for this proven instruction region only | 1 |
| `pickup_idle_actor_after_turn_r2` | `ctx->r2` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_idle_actor_before_turn_r2` | `ctx->r2` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_key_crystal_contact_quest_mode_r3` | `ctx->r3` | Original contact mode only through the mode3 comparison; register becomes scene identity afterwards | 1 |
| `pickup_kind111_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC770; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind112_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC78C; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind113_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC7A8; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind114_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC7E0; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind115_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC754; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind116_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC7C4; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind117_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind117_quest_parameter_address_r16` | `ctx->r16` | Original s0 хранит адрес8021BE10 от завершения построения адреса до восстановления caller s0; сам flag читается как halfword. | 1 |
| `pickup_kind119_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind119_quest_parameter_address_r16` | `ctx->r16` | Original s0 хранит адрес8021BE10 от завершения построения адреса до восстановления caller s0; сам flag читается как halfword. | 1 |
| `pickup_kind120_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind121_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind122_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind122_quest_parameter_address_r16` | `ctx->r16` | Original s0 хранит адрес8021BE10 от завершения построения адреса до восстановления caller s0; сам flag читается как halfword. | 1 |
| `pickup_kind124_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC8B0; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind125_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind125_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC8CC; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind126_contact_mode_r3` | `ctx->r3` | Original contact mode из8021BCE4 только в доказанных проверках mode1/<2/mode3; дальнейшая роль регистра не переименовывается. | 1 |
| `pickup_kind127_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC904; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_kind128_descriptor_address_r4` | `ctx->r4` | Original a0 при сборке и передаче точного ROM descriptor801FC920; за пределами этого вызова исходное имя регистра сохранено. | 1 |
| `pickup_parent_transform_matrix_r17` | `ctx->r17` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_rise_initial_actor_r3` | `ctx->r3` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_rise_initial_physics_r3` | `ctx->r3` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_scale_actor_r4` | `ctx->r4` | Original a0 указывает на actor весь body; stores18/1C/20 задают XYZ scale. | 1 |
| `pickup_scale_kind_r3` | `ctx->r3` | Original v1 хранит kind из physical20 для проверки диапазона и табличного индекса; далее не переназначается. | 1 |
| `pickup_scatter_height_actor_r16` | `ctx->r16` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_second_arc_actor_r16` | `ctx->r16` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_shrink_actor_r3` | `ctx->r3` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_shrink_physics_r4` | `ctx->r4` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_spawn_linked_effect_index_r17` | `ctx->r17` | Original s1 теперь индекс0..2: проверяет свободный physical80 link, задаёт индекс связанного child и считает три места. | 1 |
| `pickup_spawn_linked_effect_selector_r17` | `ctx->r17` | Original s1 хранит второй аргумент после копии a1; сравнивается signed low16 с1. Область заканчивается до новой роли loop index. | 1 |
| `pickup_spin_initial_physics_r3` | `ctx->r3` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_spin_opacity_actor_r4` | `ctx->r4` | Exact original register storage in the proven instruction region; register reuse outside the region retains the original name. | 1 |
| `pickup_star_piece_contact_quest_mode_r3` | `ctx->r3` | Original contact mode only through the mode3 comparison; register becomes scene identity afterwards | 1 |
| `player_actor_r2` | `ctx->r2` | Текущий actor персонажа из8021D49C | 58 |
| `player_actor_r3` | `ctx->r3` | Текущий actor персонажа из8021D49C | 15 |
| `player_actor_r4` | `ctx->r4` | Текущий actor персонажа из8021D49C | 3 |
| `player_actor_r5` | `ctx->r5` | Текущий actor персонажа из8021D49C | 2 |
| `player_control_r2` | `ctx->r2` | Состояние управления персонажем из8021D6C0 | 23 |
| `player_control_r3` | `ctx->r3` | Состояние управления персонажем из8021D6C0 | 65 |
| `player_control_r4` | `ctx->r4` | Состояние управления персонажем из8021D6C0 | 4 |
| `player_control_r6` | `ctx->r6` | Состояние управления персонажем из8021D6C0 | 1 |
| `player_physics_r16` | `ctx->r16` | Физическое состояние персонажа из8021D4B0 | 4 |
| `player_physics_r17` | `ctx->r17` | Физическое состояние персонажа из8021D4B0 | 2 |
| `player_physics_r2` | `ctx->r2` | Физическое состояние персонажа из8021D4B0 | 56 |
| `player_physics_r3` | `ctx->r3` | Физическое состояние персонажа из8021D4B0 | 31 |
| `player_physics_r4` | `ctx->r4` | Физическое состояние персонажа из8021D4B0 | 25 |
| `player_physics_r6` | `ctx->r6` | Физическое состояние персонажа из8021D4B0 | 1 |
| `player_physics_r8` | `ctx->r8` | Физическое состояние персонажа из8021D4B0 | 1 |
| `point_input_x_float_bits_r5` | `ctx->r5` | Float-биты входногоX до переноса в f6; позднее a1 имеет другую роль | 1 |
| `point_input_y_float_bits_r6` | `ctx->r6` | Float-биты входногоY до переноса в f8 | 1 |
| `point_input_z_float_bits_r7` | `ctx->r7` | Float-биты входногоZ до переноса в f10 | 1 |
| `point_transform_matrix_r4` | `ctx->r4` | Входная матрица; регистр сохраняет указатель во всей функции | 1 |
| `pool_blink_source_ticks` | `ctx->r2` | Signed D8 countdown до сравнения с2; следующий slti меняет рольr2. | 1 |
| `pool_opacity_command_physics` | `ctx->r3` | Физика объекта при Source byte29 check и снятии opacity marker2A. | 1 |
| `pool_private_actor_kind` | `ctx->r2` | Signed kind20 до вычисления84-byte callback table offset. | 1 |
| `pool_private_callback_offset` | `ctx->r3` | 84*kind для адреса update entry80202064. | 1 |
| `pool_private_dispatch_physics` | `ctx->r3` | Текущая физика при повторном byte29 check и чтении signed kind20. | 1 |
| `pool_private_update_callback` | `ctx->r2` | Точный callback pointer из таблицы; вызывает LOOKUP_FUNC. | 1 |
| `pool_search_actor_r3` | `ctx->r3` | Actor record cursor while testing occupied states | 1 |
| `pool_search_index_r4` | `ctx->r4` | Parent lower bound and advancing free-slot search index | 1 |
| `pool_skip_private_update_byte29` | `ctx->r2` | Значение byte29; ненулевой флаг пропускает private update объекта. | 1 |
| `powan_actor_r16_802211A8` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_80221FB0` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_80222044` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_80222074` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_802239AC` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_80223A88` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r16_80223B78` | `ctx->r16` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_8022096C` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80220CD8` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80220D0C` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80220D78` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80221004` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802212AC` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80221328` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80221374` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_8022146C` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802215CC` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802216E4` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80221BE4` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802221C0` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80222460` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80222558` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802229FC` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80222A24` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80222A80` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80223324` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_8022334C` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802233A8` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_80223574` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r2_802239F4` | `ctx->r2` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80220B20` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80220C1C` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802211F0` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80221490` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802214EC` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_8022169C` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80221774` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802220FC` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_8022224C` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80222328` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80222344` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80222514` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802225E8` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802228E4` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223014` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_8022307C` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223108` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_802231EC` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223208` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223280` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223598` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223700` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r3_80223724` | `ctx->r3` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_8022093C` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80220FD4` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_8022150C` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_802217C0` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_802217EC` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80221850` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80221BB4` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80221CB8` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_802223AC` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80222854` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80222C7C` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80222DD4` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80222EC8` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_802235FC` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80223970` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_actor_r4_80223B98` | `ctx->r4` | Actor pose/state из global8021BDEC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r16_80223B28` | `ctx->r16` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80220AC8` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80220B28` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80220C14` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221430` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221488` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221504` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_802216AC` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_8022176C` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_802217B8` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221C70` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221D3C` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80221E4C` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222244` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222304` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_8022233C` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_802225E0` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222948` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222AD8` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222D00` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222DCC` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80222FD4` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223110` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_802231C8` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223200` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223288` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223590` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223608` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_8022371C` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_802238C0` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r2_80223968` | `ctx->r2` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220974` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220994` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220A04` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220A9C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220AE0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220B88` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220BE0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220C44` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220C80` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80220D44` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022100C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022102C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221084` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221100` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221230` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802212D4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221330` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022137C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802213D8` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802215A4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221604` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221660` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221798` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802217E4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221874` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802218AC` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221BEC` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221C0C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221CC0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221D98` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221DE0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80221F90` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222064` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222140` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222180` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802221E4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222368` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802223A4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222438` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222498` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022260C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222904` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222AC4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222BF8` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222C34` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222C84` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222D54` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222DAC` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222E88` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80222ED0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802230CC` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022322C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802232A4` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802234F8` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_8022357C` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802235C0` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80223748` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_80223884` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r3_802239FC` | `ctx->r3` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802211F8` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802212B4` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802214B8` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80221E60` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80221EF0` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80222104` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802221C8` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802222AC` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802224F4` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80222E20` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_8022300C` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80223170` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80223550` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80223684` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_802236DC` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_8022390C` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `powan_physics_r4_80223AAC` | `ctx->r4` | Physical private state/counters из global8021BCDC; имя ограничено участком до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r16_8022E4FC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022D8A4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022DAA8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022DBF0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022DDE8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E07C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E114` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E170` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E2BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E398` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E464` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r2_8022E538` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r3_8022D7A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r3_8022D940` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r3_8022DCAC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r3_8022DFF8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r3_8022E1BC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_actor_r4_8022DD98` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022D7BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022D8F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022D98C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DB20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DB88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DBA4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DCB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DD24` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DD90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DE00` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022DE80` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E010` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E208` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E244` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E2F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E364` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E3B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r2_8022E438` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D7CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D7E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D834` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D8D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D96C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022D9C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022DA7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E020` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E038` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E084` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E0A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E140` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E160` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E1E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E220` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r3_8022E384` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022D7AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022D8AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022D908` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022DA90` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022DD68` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022E000` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022E11C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `radicon_physics_r4_8022E184` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `reset_scene_actor_r16` | `ctx->r16` | Actor argument для всех очищаемых полей и исходных axis/event callbacks | 1 |
| `resolved_contact_parent_r6` | `ctx->r6` | Contact owner after resolving the optional attachment parent | 1 |
| `retiring_actor_r17` | `ctx->r17` | Actor retained through peer cleanup, optional effects and final destructor | 1 |
| `retiring_physics_r19` | `ctx->r19` | Physics record retained until the final EA4C destruction | 1 |
| `rikurage_actor_r2_802213B0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802218F8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80221968` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802219AC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80221AC8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80221B00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80221C94` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80221F88` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80222188` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802224F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802225E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802226D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_802228D0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80222A14` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r2_80222A4C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_80221084` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_802210F4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_80221520` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_80221A34` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_802220DC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_80222980` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r3_80222C28` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r4_80221918` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r4_8022243C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_actor_r4_8022273C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022109C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802210EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802211D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221214` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802212B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802212EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221330` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022137C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221410` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022153C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802215A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221630` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221648` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802216F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221814` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221860` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802219F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221A7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221D44` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221DE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221E98` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221ED4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221F18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80221FEC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802221A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802221E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222200` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222350` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802223A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222434` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022247C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222558` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802225A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222608` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222644` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222660` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022279C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802227CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222828` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222840` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222884` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_8022291C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_802229C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r2_80222B74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802210AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802210C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221140` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221160` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802213DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802216D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221774` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802218F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802219D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221C9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221D10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221D78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80221FB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_8022214C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_8022226C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80222524` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_802228F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80222B4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r3_80222B9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_8022108C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802213B8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_80221478` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802214D4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802217BC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802218BC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_80221F90` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_80222058` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802220B4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_80222280` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802222F8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_80222400` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_802224FC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `rikurage_physics_r4_8022266C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `room_id_r4` | `ctx->r4` | ID текущей комнаты; сохраняется до первых вызовов проверки сюжетных флагов | 1 |
| `room_request_builtin_descriptor_r2` | `ctx->r2` | Completed built-in descriptor pointer until BA05C overwrites v0 | 1 |
| `room_request_descriptor_r4` | `ctx->r4` | Input pointer published to both Original room-descriptor globals | 1 |
| `room_request_id_input_r4` | `ctx->r4` | Input ID until stored; a0 later becomes descriptor pointer | 1 |
| `room_request_mode_r5` | `ctx->r5` | Input mode stored exactly, with no boolean conversion | 1 |
| `room_request_pending_one_r2` | `ctx->r2` | Exact register assigned1 and stored as pending request | 1 |
| `room_transition_descriptor_r2` | `ctx->r2` | Указатель pending descriptor после cleanup, до замены v0 результатом сравнения | 1 |
| `room_transition_handled_r2` | `ctx->r2` | Результат BA0F4: nonzero завершает этот world update до остальных callbacks | 1 |
| `room_transition_load_phase_r16` | `ctx->r16` | Та же загруженная стадия на отдельном пути stage2; сравнение с mode2 | 1 |
| `room_transition_mode_r3` | `ctx->r3` | Исходный request mode после загрузки; выбор ветки до calls, меняющих caller-saved регистры | 1 |
| `room_transition_pending_phase_r16` | `ctx->r16` | Загруженная исходная стадия pending request; далее s0 переиспользуется для другой роли | 1 |
| `room_transition_requested_id_r3` | `ctx->r3` | ID комнаты из descriptor для проверки диапазона и вычисления смещения таблицы | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8022ED94` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8022EF48` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_802312CC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023DDDC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023DE88` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023E294` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023E7A8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023E7B0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023EA98` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_alpha_or_amplitude_proposal_f0_8023EBEC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_8022E430` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_8022F97C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_802325F0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_802327FC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_802328B8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_8023E18C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f0_8023E5C0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_heading_proposal_f4_8023F518` | `ctx->f4` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_pitch_proposal_f0_8023E7D4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_pitch_proposal_f0_8023F658` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_radius_proposal_f0_8023DFF8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_radius_proposal_f2_8023DA68` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_radius_proposal_f2_8023DC70` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_radius_proposal_f2_8023E264` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_roll_or_velocity_proposal_f0_8023D884` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_roll_or_velocity_proposal_f0_8023DE30` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_801E98A8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_801E98BC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_801E9E90` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_801E9EA4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_801EAC2C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8022FCD0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023D8C8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023DA50` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023DC60` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023DFCC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023E230` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023F520` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023F544` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f0_8023F878` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f4_8023E780` | `ctx->f4` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_or_velocity_proposal_f4_8023EBC4` | `ctx->f4` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_801E98D0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_801E9EB8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_801EAE30` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8022FCD8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023D8D4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023DA5C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023DC6C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023DFD8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023E23C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023F52C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023F54C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f0_8023F7D4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f2_8023E788` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f2_8023EBCC` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_y_proposal_f2_8023F8BC` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_801E98CC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_801E9EB4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_801EAC30` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8022FCD4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023D8CC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023DA58` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023DC68` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023DFD0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023E238` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023EA70` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023F528` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f0_8023F548` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f2_8023F88C` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f4_8023E784` | `ctx->f4` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_scale_z_proposal_f4_8023EBC8` | `ctx->f4` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_speed_proposal_f0_8023E138` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_speed_proposal_f0_8023E420` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_x_proposal_f0_801EA7DC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_x_proposal_f2_80231080` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_x_proposal_f2_8023E184` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_x_proposal_f2_8023E74C` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f0_80231974` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f0_8023D784` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f0_8023ED88` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_80231168` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023D848` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023D9F8` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023DD60` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023E314` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023E640` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_8023F588` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_y_proposal_f2_802402D0` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_z_proposal_f0_801EA7F4` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_z_proposal_f2_802310C0` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_world_z_proposal_f2_8023E78C` | `ctx->f2` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_801E9654` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_801E97F0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_801EA820` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_801EACC8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022DD34` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022DDA0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022DEF0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022E434` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022E6BC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022EB1C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022F1A0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022F980` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8022FC08` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80230400` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80230AE0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_802310F0` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80231190` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80231688` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80232070` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8023215C` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8023DE64` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8023EDA8` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_8023FEEC` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `scene_yaw_or_velocity_proposal_f0_80240144` | `ctx->f0` | Final Source arithmetic proposal at this exact ROM store. Discrete reset, contact, init and terminal setters keep their own Source semantics. | 1 |
| `script_camera_control_r16` | `ctx->r16` | Текущий camera control stride1B8 до восстановления s0. | 1 |
| `script_camera_output_r17` | `ctx->r17` | Output camera stride34 после смены роли индекса s1. | 1 |
| `script_motion_control_r4` | `ctx->r4` | Control записи движения/камеры; точное storage исходного a0, без замены register ABI. | 11 |
| `script_thread_record_r16` | `ctx->r16` | Текущая thread record stride10 в восьмипоточном Source interpreter. | 1 |
| `script_thread_wait_r3` | `ctx->r3` | Signed wait6 до перехода к opcode; scope не захватывает новые роли v1. | 1 |
| `second_shape_index_output_r17` | `ctx->r17` | Output pointer for the second colliding shape index | 1 |
| `seek_group_offset_r5` | `ctx->r5` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `seek_input_actor_r4` | `ctx->r4` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `seek_time_float_bits_r6` | `ctx->r6` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `selected_contact_pool_index_r3` | `ctx->r3` | Signed contact index before address-scale arithmetic reuses the register | 1 |
| `sentoutei_actor_r16_8022ED1C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E22C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E318` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E340` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E404` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E434` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E4B8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E5E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E688` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E764` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E78C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E7E8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E88C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022E968` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022EA10` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022EADC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022EB70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022EC00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022EC84` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022ED58` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F268` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F354` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F37C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F53C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F718` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F7C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F89C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F8C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F920` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022F9C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022FAA0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022FB48` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022FC14` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022FCA8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_8022FD38` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r2_80230020` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022E050` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022E390` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022E9E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022EF90` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F09C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F0E8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F3D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F470` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F4AC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022F684` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022FB20` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022FE84` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r3_8022FF60` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_actor_r4_8022FEDC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r16_8022E14C` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r16_8022F188` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E068` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E0B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E264` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E2E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E388` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E44C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E628` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E6A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E8C0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E934` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E9B4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022E9E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022EA88` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022EFA8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F0E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F2A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F320` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F3C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F4D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F760` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F7DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022F9F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FA6C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FAEC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FB18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FBC0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FEE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8022FF58` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r2_8023004C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E078` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E090` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E0D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E11C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E13C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E304` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E368` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E500` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E554` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E5D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E600` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E640` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E728` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E954` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022E98C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022EBE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022EFB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F040` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F0BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F114` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F158` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F178` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F340` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F3A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F584` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F5F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F668` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F704` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F738` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F778` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022F860` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022FA8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022FAC4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022FD20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r3_8022FEB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E058` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E188` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E348` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E3B0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E43C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E690` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022E6E0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022EA9C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022EF98` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F07C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F1C4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F384` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F418` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F478` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F4B4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F7C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022F818` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022FBD4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_8022FE8C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r4_80230028` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sentoutei_physics_r6_8022FF0C` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_actor_r2_801E565C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_actor_r2_801E56A8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_actor_r2_801E56C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_physics_r3_801E559C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_physics_r3_801E5644` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_physics_r3_801E56CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `sharedeffects_physics_r3_801E57C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `slot09_kind24_bind_clip6_once_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind24_contact_mode_r3` | `ctx->r3` | Original contact mode through mode1/3/4 comparisons only; no alias after register reuse | 1 |
| `slot09_kind24_dialogue_restore_resume_publish_init_physics_r3` | `ctx->r3` | Physics pointer only for the initial latch28 read/write; no alias after later register reuse | 1 |
| `slot09_kind24_dispatch_state_r3` | `ctx->r3` | Actor state only through the switch comparisons, before a private callback can reuse v1 | 1 |
| `slot09_kind24_patrol_z_reverse_at100_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind24_wait40_clip_then_patrol_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind24_wait_count_r3` | `ctx->r3` | Signed half wait counter only from increment to threshold comparisons; v1 becomes an actor pointer later | 1 |
| `slot09_kind25_bind_idle_clip0_once_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind25_contact_mode_r3` | `ctx->r3` | Original contact mode through mode1/3/4 comparisons only; no alias after register reuse | 1 |
| `slot09_kind25_dialogue_turn_clip7_restore_patrol101_init_physics_r3` | `ctx->r3` | Physics pointer only for the initial latch28 read/write; no alias after later register reuse | 1 |
| `slot09_kind25_dialogue_turn_six_count_restore_init_physics_r3` | `ctx->r3` | Physics pointer only for the initial latch28 read/write; no alias after later register reuse | 1 |
| `slot09_kind25_dispatch_state_r3` | `ctx->r3` | Actor state only through the switch comparisons, before a private callback can reuse v1 | 1 |
| `slot09_kind25_patrol_x_reverse_at100_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind25_wait40_clip_then_patrol101_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind25_wait_count_r3` | `ctx->r3` | Signed half wait counter only from increment to threshold comparisons; v1 becomes an actor pointer later | 1 |
| `slot09_kind26_bind_idle_clip0_once_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind26_dialogue_restore_scene_quest_spawn_init_physics_r3` | `ctx->r3` | Physics pointer only for the initial latch28 read/write; no alias after later register reuse | 1 |
| `slot09_kind26_dispatch_state_r3` | `ctx->r3` | Actor state only through the switch comparisons, before a private callback can reuse v1 | 1 |
| `slot09_kind26_patrol_x_reverse_at50_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind26_wait40_clip_then_patrol101_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind26_wait_count_r3` | `ctx->r3` | Signed half wait counter only from increment to threshold comparisons; v1 becomes an actor pointer later | 1 |
| `slot09_kind27_bind_idle_clip0_once_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind27_dialogue_turn_restore_release_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind27_dispatch_state_r3` | `ctx->r3` | Actor state only through the switch comparisons, before a private callback can reuse v1 | 1 |
| `slot09_kind28_dialogue_align_six_count_restore_release_init_physics_r3` | `ctx->r3` | Physics only for the initial latch28 check/write, before subsequent callbacks or register reuse | 1 |
| `slot09_kind28_dispatch_state_r3` | `ctx->r3` | Actor state only through the switch comparisons, before a private callback can reuse v1 | 1 |
| `slot09_wait_counter_physics_r3` | `ctx->r3` | Physics for count increment/comparison and completed wait latch clear | 1 |
| `slot09_wait_init_physics_r3` | `ctx->r3` | Physics for the once-only wait latch and count reset | 1 |
| `soundorago_actor_r16_8023DA54` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r16_8023DCA4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r16_8023E39C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r16_8023E4A8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023DACC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023DCE8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023DD74` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023DE68` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023DFAC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023E3D4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023E52C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023E8CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r2_8023EE00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r3_8023D904` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r3_8023E128` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r3_8023E7A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r4_8023DA20` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r4_8023DA90` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r4_8023E034` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r4_8023E2AC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r4_8023EB7C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_actor_r5_8023DBEC` | `ctx->r5` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r16_8023DB98` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r16_8023E980` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r16_8023EAC4` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r17_8023DB4C` | `ctx->r17` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r17_8023DC60` | `ctx->r17` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023D924` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023DA18` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023DBC0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023DC3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023DC94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023DF10` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E134` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E1A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E360` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E46C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E7C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E9A4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023E9C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023EB04` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023EC3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r2_8023EEB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023D93C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023D9CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023D9F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DAF4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DC1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DDD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DE38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DF54` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023DFDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E0A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E174` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E330` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E43C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E7DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E86C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E894` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E8BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023E9EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EB38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EC0C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EC78` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EE38` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EE5C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EE9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r3_8023EF10` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023D90C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023DAD4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023DD08` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023E7AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023E90C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023ED60` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r4_8023EED0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r5_8023E2A0` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `soundorago_physics_r5_8023EB74` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `special24_completed_actor_r3` | `ctx->r3` | Exact Original register storage in this proven instruction region only | 1 |
| `special24_initial_physics_r3` | `ctx->r3` | Exact Original register storage in this proven instruction region only | 1 |
| `special28_initial_physics_r3` | `ctx->r3` | Exact Original register storage in this proven instruction region only | 1 |
| `special28_return_physics_r3` | `ctx->r3` | Exact Original register storage in this proven instruction region only | 1 |
| `sprite_record_r16` | `ctx->r16` | Выбранная sprite pool record после преобразования индекса в адрес | 1 |
| `sprite_record_r4` | `ctx->r4` | Восьмибайтная запись sprite pool, переданная параметром | 1 |
| `state2_update_count_r2` | `ctx->r2` | Счётчик обновлений state2 до превращения регистра в результат сравнения | 1 |
| `story_completion_word_r4` | `ctx->r4` | Exact Original register storage in this proven instruction region only | 1 |
| `story_constructor_variant_r18` | `ctx->r18` | Resolved variant0/1/2, retained until the constructor epilogue | 1 |
| `story_profile_r17` | `ctx->r17` | Story-selected profile2 or0/1 retained across allocator calls | 8 |
| `supaizan_actor_r16_8021DD0C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r16_8021DE38` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r16_8021F438` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021D80C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021DAEC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021DED4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021DF78` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021DFB0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E038` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E16C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E1A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E1D8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E26C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E3C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E404` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E4A8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E4E4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E514` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021E6FC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021EAFC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021EBE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021EC7C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021ECD8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021EE00` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021EEE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021F0A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021F0C8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021F12C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021F26C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r2_8021F3A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021D960` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021DB9C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021DFFC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021E2C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021E9A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021EA3C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021F2C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021F4C8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r3_8021F7B0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021D98C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021DA2C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021DA88` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021E09C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021E5DC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021EA68` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021F358` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_actor_r4_8021F68C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021D8D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DA24` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DA80` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DBE0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DE20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DF54` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021DFF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E094` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E484` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E594` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E7C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E7FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021E9BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EA04` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EB24` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EB48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EE3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EEB0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EF30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021EFC0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F2BC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F310` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F4E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F578` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F660` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F680` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r2_8021F7D0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021D814` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021D894` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021D8AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021D984` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021D9F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021DD04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021DEDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E040` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E068` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E354` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E3A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E40C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E51C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E56C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E5E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E6DC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E7A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E9CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021E9E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EA60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EAD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EB34` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EBDC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021ECE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021ED50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EED0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021EF08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F00C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F05C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F180` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F200` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F2E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F360` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F4F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F50C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F750` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r3_8021F798` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021D968` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021DAC4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021DB94` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021E304` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021E3C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021E9AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021EA44` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021F1AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021F4D0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r4_8021F604` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r5_8021F534` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `supaizan_physics_r6_8021EB80` | `ctx->r6` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r16_8023D880` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r16_8023DB90` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r16_8023E194` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r16_8023E3F0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r2_8023D778` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r2_8023DACC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r2_8023DBE4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r2_8023E444` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_actor_r3_8023E370` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_80231000` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023D704` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023D790` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023D850` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023DB74` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023E164` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023E388` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r2_8023E3D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023D6E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023D82C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023DAD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023DB34` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023DB50` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023E140` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023E398` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r3_8023E3B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `t00t17iwa_physics_r4_8023E378` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r16_80221D40` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r16_80221F40` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r16_80222634` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r16_8022285C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80221A70` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222008` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222040` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222068` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222118` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222140` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222374` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222924` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_8022295C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222984` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r2_80222BC0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80221A00` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80221B88` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80221C78` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80221CB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80221E38` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_802222D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_80222574` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r3_802225B4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r4_802222FC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r4_80222450` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r4_80222754` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_actor_r4_80222C5C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r16_80221A78` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r16_8022237C` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_802219DC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221A48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221B44` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221B80` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221D34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221DBC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221E30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221EB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221F34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80221FBC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_802220D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_8022234C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_8022240C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80222628` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_802226B0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80222724` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_802227D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80222850` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_802228D8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80222AE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r2_80222B8C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221A18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221AD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221AF0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221B08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221BE8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221D08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221D20` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221D98` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221DD8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221E74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221F1C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80221F94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222098` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222178` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802222F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_8022231C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802223D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802223F4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222448` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802224E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802225FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222614` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_8022268C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802226CC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_8022274C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222798` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222838` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_802228B0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222A08` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222AB8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222B6C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r3_80222C7C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221C3C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221C70` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221CB0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221E4C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221EF8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80221FF0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80222538` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_8022256C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_802225AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80222770` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80222814` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tamabonga_physics_r4_80222BA8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `target_actor_r5` | `ctx->r5` | Input actor whose physics update permission changes | 1 |
| `target_heading_f14` | `ctx->f14` | Входной target angle до аргумента нормализации разницы | 1 |
| `tatsunoorgo_actor_r2_80222E8C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80222EB4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80222F10` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80222F68` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80223164` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_802232A4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_802232CC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80223328` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_80223448` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_8022347C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_802237D8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r2_8022382C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r3_80222DB8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r3_802233D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r3_80223580` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r3_802236A8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r3_8022396C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r4_80222FDC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_actor_r4_802231D8` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r16_80223020` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80222DD0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80222E28` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80222FD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_802230F4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_802231CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_802233C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80223428` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80223598` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80223664` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_80223780` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r2_8022398C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80222DE0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80222DF8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80222E60` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80222FA8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802230A4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_8022311C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_8022316C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80223278` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802233F8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802235A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802235C0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802235E4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80223604` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802237C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_80223890` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_802238D0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r3_8022390C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r4_80222DC0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r4_80223588` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r4_80223954` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `tatsunoorgo_physics_r5_802231E0` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r16_802205D4` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r16_802207E0` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r16_802209FC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80220338` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80220838` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80220B30` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80220B50` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80221158` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80221178` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_802215DC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80221604` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r2_80221854` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802202D8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_8022043C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802204F8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80220640` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_8022067C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80220A5C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80220C58` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80220CA4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80220F3C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802210B0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802210CC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_80221268` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802212B0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802214A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_8022158C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_802217D0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r3_8022186C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r4_802206DC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r4_802208D4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r4_80220D8C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r4_80220DBC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_actor_r4_80221388` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r16_80220330` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802203F8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220434` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220510` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802205C8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802206CC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_8022075C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802207D4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802208C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220978` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802209F0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220AF0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220C9C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220CE8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220D84` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80220F8C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80221000` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802210C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80221118` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802212A8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802212FC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80221380` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_802216EC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_8022170C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r2_80221888` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802202F0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220308` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802203A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802203C8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_8022049C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220530` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_8022059C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802205B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802206A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220700` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802207BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_8022087C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802208A0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802208FC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802209D8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220A9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220AC0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220CC8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220F54` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80220F6C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802210E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802212D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_80221350` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802214BC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802216A8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r3_802217EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_802204F0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_8022062C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220674` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220798` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220854` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_802209B4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220A54` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220A74` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80220C60` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_802210A8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80221270` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_8022151C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_802215B8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `thunderdischarge_physics_r4_80221820` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `title_letter_record_r3` | `ctx->r3` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 2 |
| `title_letter_record_r4` | `ctx->r4` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `title_letter_record_r5` | `ctx->r5` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `translation_matrix_output_r4` | `ctx->r4` | Указатель на64-байтовую выходную матрицу | 1 |
| `translation_x_float_bits_r5` | `ctx->r5` | Float-биты переносаX; сохраняются без числового преобразования | 1 |
| `translation_y_float_bits_r6` | `ctx->r6` | Float-биты переносаY | 1 |
| `translation_z_float_bits_r7` | `ctx->r7` | Float-биты переносаZ | 1 |
| `twinface_actor_r16_8022DCFC` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022DDDC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022DE58` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022DEF0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022DF18` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022E2BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022E6E0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022E7BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022E9F0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r2_8022EBD0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022DC40` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E1C4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E31C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E3A4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E3C8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E3EC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E588` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022E8B0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r3_8022EA28` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r4_8022E054` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r4_8022E5F0` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r4_8022E618` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r4_8022E688` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_actor_r4_8022EA74` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022DCF4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022DD3C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022DE34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022E248` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022E44C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022E5E0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022E7E8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022E90C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022EA20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022EA7C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022EB30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r2_8022EC00` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DCB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DCD0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DD58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DDA0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DE04` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022DEC4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E174` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E21C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E5B8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E670` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E8E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r3_8022E924` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022DDE4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022E0AC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022E1CC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022E590` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022E7C4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022E8B8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r4_8022EBD8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r5_8022E020` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `twinface_physics_r5_8022E35C` | `ctx->r5` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `update_permission_r16` | `ctx->r16` | Saved permission command used for both pools and player state | 1 |
| `vertical_collision_actor_r17` | `ctx->r17` | Exact Original register storage for this proven instruction region only | 1 |
| `vertical_collision_physics_r16` | `ctx->r16` | Exact Original register storage for this proven instruction region only | 1 |
| `vertical_parameters_physics_r2` | `ctx->r2` | Exact Original register storage for this proven instruction region only | 1 |
| `widget_draw_slot_r18` | `ctx->r18` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `widget_record_r16` | `ctx->r16` | Reviewed UI slot/record role restricted to the exact Original instruction interval | 1 |
| `world_actor_r3` | `ctx->r3` | Текущий обновляемый объект мира из8021BDEC | 1 |
| `world_actor_r4` | `ctx->r4` | Текущий обновляемый объект мира из8021BDEC | 2 |
| `yadokaringer_actor_r16_8021EC68` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r16_8021F0C8` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r16_8021F42C` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r16_8021FE34` | `ctx->r16` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021EB90` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021ECBC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021EE28` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021EF2C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F014` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F118` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F1C4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F3B4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F9A0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021F9F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021FD5C` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_8021FE88` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_802205C0` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_802205F4` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220644` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220708` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_802207BC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220800` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220B58` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220DA8` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r2_80220DFC` | `ctx->r2` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021EABC` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021F6A8` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021F820` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021F8A0` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021FB0C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8021FC7C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_8022054C` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_80220A74` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r3_80220EF4` | `ctx->r3` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_8021F288` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_8021F54C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_8021F740` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_8021FCEC` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_8022094C` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_actor_r4_80220AF4` | `ctx->r4` | Source указатель actor из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r16_8021F92C` | `ctx->r16` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021EAD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021EB2C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021EBC8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021EC48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021ED48` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F164` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F20C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F4AC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F584` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F6C4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F718` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F834` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021F96C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FB34` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FC94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FCE4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FD94` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FE14` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8021FF0C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8022012C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_802201E4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220444` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_8022049C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220544` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_802206A0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_802206B8` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220914` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220A8C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220B78` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220BB0` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220BD4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220C0C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220C90` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220CB4` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220CFC` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220D30` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220D5C` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r2_80220F20` | `ctx->r2` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021EAE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021EAFC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021EB4C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021ED74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021EE8C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021EF74` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F04C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F0B4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F100` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F13C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F2E0` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F3AC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F480` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F6D4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F6EC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F738` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021F98C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FAAC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FCA4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FCBC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FD18` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FEEC` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8021FFD4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8022001C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_802200E8` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220224` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_8022028C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220374` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220584` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220720` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220744` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_802207C4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220A9C` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220AB4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220AE4` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220C28` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220D94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220E58` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r3_80220E94` | `ctx->r3` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021EAC4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021EDA4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021EDD0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021F6B0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021F75C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021F8C8` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021FAF4` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021FC84` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8021FF40` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_802203EC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_802204E0` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_8022062C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_80220A7C` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
| `yadokaringer_physics_r4_80220EDC` | `ctx->r4` | Source указатель physics из global8021BCDC/8021BDEC; до следующей записи регистра/внешнего вызова | 1 |
