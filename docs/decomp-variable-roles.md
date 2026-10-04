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
| `axis_rotation_angle_float_bits_r5` | `ctx->r5` | Float-биты угла до переноса в f20; затем a1 становится указателем компоненты оси | 1 |
| `axis_rotation_output_matrix_r16` | `ctx->r16` | Выходная матрица между сохранением a0 в s0 и восстановлением старого s0 | 1 |
| `axis_rotation_x_float_bits_r6` | `ctx->r6` | Float-биты осиX до сохранения в scratch-аргумент нормализации | 1 |
| `axis_rotation_y_float_bits_r7` | `ctx->r7` | Float-биты осиY до delay-slot сохранения перед нормализацией | 1 |
| `bobbing_actor_r16` | `ctx->r16` | Actor retained across the sine and angle normalization calls | 1 |
| `candidate_contact_physics_r4` | `ctx->r4` | Candidate physics after the actor argument is replaced by its physics pointer | 1 |
| `cat_contact_mode_r3` | `ctx->r3` | Shared contact mode before the checked sound command | 1 |
| `character_status_r3` | `ctx->r3` | Байтовая запись здоровья текущего персонажа из8021D4A8 | 1 |
| `character_status_r4` | `ctx->r4` | Байтовая запись здоровья текущего персонажа из8021D4A8 | 1 |
| `city_wait_count_r3` | `ctx->r3` | Private signed wait counter after its lhu read, increment/store and sign extension; before v1 becomes actor pointer | 8 |
| `constructed_common_actor_r17` | `ctx->r17` | Actor выбранного common slot: его новый physics уже опубликован, далее передаётся parser и сбросу transform | 1 |
| `constructed_common_physics_r16` | `ctx->r16` | Новый allocation F4 именно этой итерации, сохранённый в actor98 перед вызовом parser | 1 |
| `constructed_parent_actor_r16` | `ctx->r16` | New parent actor, possibly null, retained through sprite/child initialization | 1 |
| `contact_candidate_actor_r16` | `ctx->r16` | Advancing world actor cursor before projectile/pair loops reuse the register | 1 |
| `contact_candidate_pool_index_r17` | `ctx->r17` | Pool index30..229 during the player contact search | 1 |
| `contact_first_actor_r21` | `ctx->r21` | First actor retained through geometry, reciprocal slots and contact angles | 1 |
| `contact_mode_r3` | `ctx->r3` | Contact mode loaded from the shared event global before dispatch | 1 |
| `contact_mode_r3` | `ctx->r3` | Режим контакта, загруженный из8021BCE4 до выбора ветки | 1 |
| `contact_parent_actor_r5` | `ctx->r5` | Parent actor whose dialogue guard and state write precede the game-flag call | 1 |
| `contact_player_actor_r18` | `ctx->r18` | Player actor during the player-versus-world prefix only | 1 |
| `contact_player_physics_r5` | `ctx->r5` | Player physics read for cooldown and existing contact slot | 1 |
| `contact_second_actor_r20` | `ctx->r20` | Second actor retained through reciprocal contact publication | 1 |
| `controller_index_r16` | `ctx->r16` | Индекс контроллера; не указатель actor. Значение входного a0 сохранено в s0. | 1 |
| `counter_bank3678_delta_r5` | `ctx->r5` | Exact Original register storage in this proven instruction region only | 1 |
| `counter_bank3678_index_r4` | `ctx->r4` | Exact Original register storage in this proven instruction region only | 1 |
| `created_npc_actor_r16` | `ctx->r16` | Allocated actor returned by descriptor construction, including the checked null-allocation path | 8 |
| `created_npc_physics_r18` | `ctx->r18` | Physics record of the successfully created actor | 8 |
| `current_heading_f12` | `ctx->f12` | Входной current angle до перезаписи f12 целевым углом | 1 |
| `descriptor_actor_r19` | `ctx->r19` | Actor whose resource groups and physics kind are initialized | 1 |
| `descriptor_cursor_r17` | `ctx->r17` | Advancing descriptor stream cursor, returned after FFF5 | 1 |
| `dialogue_descriptor_r4` | `ctx->r4` | Input descriptor until a0 is reused for runtime parameters | 1 |
| `dialogue_entry_physics_r3` | `ctx->r3` | Physics record used only for the initial dialogue latch read and write; v1 changes role afterward | 8 |
| `dialogue_state2_r16` | `ctx->r16` | State2 constant preserved across the dialogue call | 1 |
| `drive_command_r4` | `ctx->r4` | Входная команда scalar drive; сохранена до конца функции. | 1 |
| `first_shape_index_output_r16` | `ctx->r16` | Output pointer for the first colliding shape index | 1 |
| `flash_delay_count_physics_r3` | `ctx->r3` | Physics для чтения/записи count34; следующий instruction заменяет этот регистр signed selector | 1 |
| `flash_delay_initial_physics_r3` | `ctx->r3` | Physics текущего эффекта: чтение и установка однократного init latch28 | 1 |
| `flash_motion_initial_physics_r3` | `ctx->r3` | Physics эффекта до вызова вертикального impulse: selector44, opacity58 и radius5C | 1 |
| `flash_signed_delay_r3` | `ctx->r3` | Signed selector44; количество задержки равно его модулю, отрицательные варианты считаются отдельной ветвью | 1 |
| `flash_transition_actor_r3` | `ctx->r3` | Actor эффекта при публикации uniform scale1 и state10; здесь регистр уже не physics и не selector | 1 |
| `game_counter_delta_r5` | `ctx->r5` | Signed input delta consumed by the original byte addition | 1 |
| `game_counter_index_r4` | `ctx->r4` | Input counter index retained without register reuse | 2 |
| `health_delta_r4` | `ctx->r4` | Входная прибавка к здоровью; r4 далее переиспользуется как указатель status. | 1 |
| `interaction_player_state_r3` | `ctx->r3` | Player state retained across the Original interaction state gates | 1 |
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
| `reset_scene_actor_r16` | `ctx->r16` | Actor argument для всех очищаемых полей и исходных axis/event callbacks | 1 |
| `resolved_contact_parent_r6` | `ctx->r6` | Contact owner after resolving the optional attachment parent | 1 |
| `retiring_actor_r17` | `ctx->r17` | Actor retained through peer cleanup, optional effects and final destructor | 1 |
| `retiring_physics_r19` | `ctx->r19` | Physics record retained until the final EA4C destruction | 1 |
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
| `second_shape_index_output_r17` | `ctx->r17` | Output pointer for the second colliding shape index | 1 |
| `seek_group_offset_r5` | `ctx->r5` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `seek_input_actor_r4` | `ctx->r4` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `seek_time_float_bits_r6` | `ctx->r6` | Verified role only within this exact Original instruction region; register reuse elsewhere retains ctx register name. | 1 |
| `selected_contact_pool_index_r3` | `ctx->r3` | Signed contact index before address-scale arithmetic reuses the register | 1 |
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
| `target_actor_r5` | `ctx->r5` | Input actor whose physics update permission changes | 1 |
| `target_heading_f14` | `ctx->f14` | Входной target angle до аргумента нормализации разницы | 1 |
| `translation_matrix_output_r4` | `ctx->r4` | Указатель на64-байтовую выходную матрицу | 1 |
| `translation_x_float_bits_r5` | `ctx->r5` | Float-биты переносаX; сохраняются без числового преобразования | 1 |
| `translation_y_float_bits_r6` | `ctx->r6` | Float-биты переносаY | 1 |
| `translation_z_float_bits_r7` | `ctx->r7` | Float-биты переносаZ | 1 |
| `update_permission_r16` | `ctx->r16` | Saved permission command used for both pools and player state | 1 |
| `vertical_collision_actor_r17` | `ctx->r17` | Exact Original register storage for this proven instruction region only | 1 |
| `vertical_collision_physics_r16` | `ctx->r16` | Exact Original register storage for this proven instruction region only | 1 |
| `vertical_parameters_physics_r2` | `ctx->r2` | Exact Original register storage for this proven instruction region only | 1 |
| `world_actor_r3` | `ctx->r3` | Текущий обновляемый объект мира из8021BDEC | 1 |
| `world_actor_r4` | `ctx->r4` | Текущий обновляемый объект мира из8021BDEC | 2 |
