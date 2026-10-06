# Исправления после пользовательской проверки046

База: main288ff52, Windows046. Сначала весь связанный код/имена/документы,
затем необходимые scoped проверки и один Windows047. Часовой full-player
не запускать повторно; выполненные независимые proofs сохраняют своё
происхождение. Игру/renderer/GPU локально не запускать.

Windows047 сохранён;31 completed CPU checks PASS, full-player USER_SKIP.
Все сообщения вошли в реализацию/аудит. Визуальное подтверждение остаётся
за пользователем; точное соответствие voice/fruit пока не установлено.

| Участок | Сообщённое поведение | Статус |
|---|---|---|
| Fade переходов уровня/режимов | Original FPS | Stable world fade eligibility implemented; loop/integration PASS, visual pending |
| Выбор персонажа/предмета/оружия | Original FPS | UI eligibility, fractional pose and real SDK matrices implemented; UI PASS, visual pending |
| 2D карта уровней | sprite выбора дрожит при движении | Stage-map fractional rectangle implemented; UI PASS, visual pending |
| Зарядка оружия | ступенчатое заполнение полоски | Source count50 to Native RDP10.2 fill; actual Source HUD/RDP case PASS, visual pending |
| Вода | ступенчатая анимация texture motion | Common byte/CI4 exact-scroll adapter implemented;35 CPU cases PASS, visual pending |
| Гонки на4 | окружение дрожит |4 post-terrain viewport origins and shared Source completion clock implemented; CPU suite PASS, visual pending |
| Самолёты на4 | модели дрожат |4 persistent visual/follower copies and viewport origins implemented; actual helpers PASS, visual pending |
| Крик во фрукты | дрожит последний кадр возврата | Terminal retreat defect corrected/proved; voice/fruit module identity and visual cause unconfirmed |
| Бомбы/4 квадрата | начальная камера Original FPS | Actual intro camera+SDK eyes/shared completion clock PASS, visual pending |
| Миниигры/движение | тень отстаёт | Mini8 getter-derived shadow copies implemented;4 participant CPU cases PASS, visual pending |
| Камера у стен | Original FPS и stutter | Temporal distance caps implemented; camera checks PASS, reported visual cause unconfirmed |

Дополнительный обход: аналогичные ветви других игровых menus/переходов,
все9 миниигр, derived world origins, camera/shadow bindings, draw-only
texture/counter equations. Нельзя выдавать список хуков за доказанный
непрерывный вывод или синтетический CPU результат за визуальное прохождение.

Дополнение пользователя: после исправлений провести общий аудит оставшихся
участков с Original FPS. Аудит — часть текущей работы, отдельный обязательный
этап до финальной передачи; одной проверки известных сообщений недостаточно.

Реализован refresh четырёх viewport origins в mini1/2 из позы того же
участника после Native terrain query. Раньше Y origin оставался Source
снимком из opaque SDK результата и мог расходиться с дробной позой/тенью.
Connected suite PASS; actual terrain/visual outcome проверяется пользователем.

Реализована отдельная Native frame/fade eligibility для стабильных world
callbacks BB730/BB848 при countdownD494>1 и активном resident fade. Count1
возвращает Source владельца loader/resource barrier. Source frozen/world
command cadence не ускоряется. Framebuffer-copy и graphics-off пути ещё
остаются отдельным нерешённым пунктом переходов, до анализа зависимостей.

Аудит SDK398B4/38ABC: первичная гипотеза об откате joint matrices
не подтвердилась. Два читателя в DC450 используют cache последней
отрисованной позы для collider offsets. Пока Native владеет всеми draw
banks, отдельного Original draw нет: откат замораживал бы pre-session
collider. Матрицы поэтому сохраняются, temporary TRS возвращаются по
прежнему контракту. Actual SDK matrix/getter case PASS. Причины
миниигровых copies/Source clock отдельно исправлены по доказанным связям.

Вода: разобран общий bitmap channel80028C74/80028D50 и world BBFF4.
ROM town frames0113..0122 —32x32 CI4, одна палитра, каждый следующий кадр
точный циклический сдвиг+2px X. Новый Native texture adapter распознаёт
именно такие authored scroll sequences, не смешивает картинки. В material
DL3A3B8 заимствует RDP tile origin10.2 на дробное Source время; copies
принадлежат свободному ticket bank, исходные bitmap/DL/timers не меняются.
CPU proof35 texture/graph/clamp/reuse/tail/bank cases PASS; visual pending.

Дополнительная проверка текстур по просьбе пользователя: прочитаны1475 BMP ресурсов из трёх ROM таблиц. Из828 небольших CI4 с16цветами найдены8 семейств с точным циклическим сдвигом (75 кадров), включая воду town. Наборы: general07E9..07EC (16x16, Y), general0877..087E (16x32, Y), bitmap00E0..00EE (64x64, Y),0113..0122 и0123..0132 (32x32, X),0447..044A,0462..0469,0564..0567 (16x32, Y). Это точное сравнение пикселей и палитр. Не все назначения/пути достижимости подтверждены: совпадение данных не доказывает, что каждый кадр реально используется каналом анимации. Найдены таблицы scene1, remaining_slot0_11 и level1_slot0; последний набор пока без доказанного Source владельца. Большие/других форматов BMP не входят в этот exact-scroll охват. Локальные ROM-dependent inventory/owners JSON остаются в build-local, извлечённые данные не отслеживаются в Git.

Обработка texture graph охватывает generated prefix и вложенные resource DE lists с общей image/tile state. Копии в tail текущего frame bank; если Source позднее занимает этот tail, перед submit ссылки/команды возвращаются к Source без падения. Cache переводов проверяет содержимое повторно при reuse resource slot, parent material state не смешивается при reuse DL. Active texture adapter CPU cases PASS; visual evidence отсутствует.

Mini8: shadow helper7570/actual getters73C0,73EC,7418 copies participant localX/Z/yaw to pool65+i. Its three stores previously had Scratch role, leaving the shadow on Source cadence. They now carry derived position and wrapped angle with existing allocation guards. Mini3 retreat5E1C now stops Native negative drift when the Source final worldX already crossed80, preserving the Original overshoot. This is a proven terminal defect; the voice/fruit game identity is still not proven.

Wall camera D7294: identified missing temporal distance caps +/-25 at7FE0/8020. Only their rate is fractional; collision ray search steps remain full spatial distances. Control0 helperC0694 is called by weaponF9 action C2978, so it is not evidence for a wall-camera fallback and its eligibility gate remains. Specific wall branches still require connected validation.

Аудит аналогичных поз после shadow fix: повторно просмотрены71 семейство
миниигровых Scratch float stores с local/world координатами. Дополнительно
найдены Mini2 постоянные copies: visual plane actors5..8 (5E74) и state0
follower actors21..24 (56C8). Шесть XYZ stores переведены в Derived с теми
же participant sources. Source-only рождения эффектов в state1/других
минииграх, reset/snap/clamp и collision commands сохраняют исходные роли;
они не должны продолжать следовать движущемуся родителю после рождения.
Проверка копий/теней включает реальные Source helpers; PASS для4 участников, включая Source-only effect birth.

Bitmap Source flags разобраны дополнительно по28C74: bit0 active, bit1
clamp; terminal FF при clamp останавливается на последнем frame, иначе
возвращается в index0. Adapter учитывает оба вида active последовательностей,
включая конечную удерживаемую текстуру; Source флаги/таймеры не изменяются.

Mini2 persistent world copies дополнительно обновляются после настоящего
Native terrain query, как viewport origins: Source opaque Y не является
непрерывным operand графа. Только reached Derived поля принимают новую
позицию, Source effect births остаются отдельным одноразовым владельцем.

World menus: Native borrowed local pose требует настоящего SDK2DA98 для
leased actors612..648. Иначе shader использовал Source matrix даже при
дробных yaw/XYZ. Матрицы пересчитываются в CPU math с заимствованным stack,
затем вместе с dirty/worldXYZ возвращаются до следующего update. Выбранная
модель648, ghost actors и оба кольца входят в общий механизм.

Аналогичный menu matrix путь найден в scene34 Selection:14B8 меняет
preview actor801724C8 localY/Z. Он подключён к тому же matrix/dirty/world
lease и настоящему2DA98, чтобы fractional pose попадала в draw и здесь.

## Общий аудит FPS после исправлений

Повторно проверены владельцы времени и пути видимой позы: world gameplay/
scripted callbacks, world menus1..3, scene34 stage-map/selection/title/files,
девять minigame ROM modules и три common пары, resident fade, charge HUD,
bitmap channel bind и material/RDP graph. Дополнительные найденные пропуски
переведены в общий код: постоянные copies самолёта/follower, shadow mini8,
post-terrain copies/origins, menu/selection local matrices и bitmap clamp/reuse.

Доказанные различия lifetimes: Source effect birth не должен постоянно
следовать родителю; joint draw matrices являются cache последней видимой
позы для collider DC450 и не возвращаются к pre-session состоянию. Эти
ветви сохранены после просмотра реальных Source consumers. Контактные
snap/reset/retire, выбор состояния/клипа, загрузка/сохранение, RNG/input/sound,
score/countdown по-прежнему Source once: это команды, а не забытая анимация.

Original guards остаются для настоящей замены ROM/resources/callbacks,
graphics-off и cross-frame framebuffer dependency. В стабильном world fade
countdown>1 можно рисовать Native; count1 передаёт loader владельца.
Четыре вспомогательные actor/resource/model control пары из046 остаются
явным Source fallback; текущая работа не заявляет их Native движение.
Authored bitmap frames с разными рисунками/палитрами не превращаются в
новые изображения. Exact-scroll adapter действует для поддерживаемых CI4
и совместимых wrapped tiles; inventory не доказывает достижимость всех
ресурсов и не заменяет визуальный обход игры.

Полный connected pre-build047 запущен один раз после связанного кода/имён,
без часового full-player по прямой просьбе пользователя. При ошибках
исправляется причина и повторяются только failed/affected scopes. Общее
завершение этого набора нельзя называть full-suite PASS из-за USER_SKIP.
Игра/renderer/GPU локально не запускаются; визуальные сообщения пользователя
подлежат повторной оценке в047, а не считаются доказанными CPU моделями.

Новый texture CPU case поймал memory-safety ошибку: unsigned offset в
MEM_BU вместе с signed KSEG base давал адрес за RDRAM. Все переменные
offsets нового adapter приведены к signed, включая index/duration и
header/palette/pixel fingerprints. Остальные новые pose/menu paths используют
проверенные word helpers либо явные signed offsets. Повторяется texture scope.

Расширенный audit всех1475 BMP (formats1/2/4/5/6, полный размер/палитра/пиксели)
нашёл ещё3 точных scroll семейства помимо первых8: general0653..0656
(format2 byte pixels,64x32, X/Y), general09CE..09D4 (CI4,3цвета,16x4, X),
bitmap0521..0528 (CI4,6цветов,32x32, Y). Итого11 семейств /94 кадра.
Общий adapter расширен на подтверждённый byte format2 и CI4 palettes1..16.
SDK282EC/28320 определяют payload/palette offsets; реальные format2 ресурсы
имеют ровно16+width*height байт. Шесть новых actual Source bind/material
случаев прошли CPU, общий texture scope34 cases PASS до последнего уточнения
пустого palette pointer дляformat2. Два BMP header-only ресурса не имеют pixel
payload; их картинки/назначение не угадываются. Runtime reachability всех
новых семейств ещё не доказана. Игра/GPU не запускались.

047: Actual charge HUD generates an RDP F6 rectangle with fractional right edge; other commands, counter/input and release/cap branches preserved. Mini2 four persistent copies and one Source birth, Mini8 four shadow/getter chains, terminal retreat and delayed intro Source completion passed. Mini3 retreat speed801C8ED0 is a runtime field initialized by7F68, not an immutable ROM coefficient.
