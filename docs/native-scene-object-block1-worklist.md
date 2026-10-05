## Test-044: весь выбранный блок1 — объекты, платформы и оставшиеся NPC

Сборка044 из generation startup-scene-block1-v4 сохранена в
D:/Games/DoraRecomp3/testbuild после29 connected CPU checks.
Реализованы289 типов,292 точных ROM-варианта:42 NPC,5 общих объектов,
129 тестовых/EV/исчезающих/damage/platform объектов и113 stage objects.
Движение, доказанные повороты/наклоны/масштаб/прозрачность и время моделей
получают Manual dt. Wait/RNG/contact/quest/clip/spawn/retirement сохраняют
Source порядок. Original20/30 и канонический ABI/ROM identity сохранены.

Связанные траектории133/527/528/564 учитывают Source порядок родителя,
epoch и lifetime. Door109 прибавляет своё перемещение к действительным
координатам игрока, сохраняя его собственное движение. Init/reset/floor и
terminal stores отменяют устаревшие математические предложения.
Исправлено знаковое вычисление адреса Fly44–46 в Native NPC повороте.
Door109 сохраняет Native обновление common-world во время своего реального
B8ED0 затемнения(D494=16), с проверкой lifetime/epoch. Игрок C8B28 остаётся
остановленным по Source правилу BB080. Устранён повторный Source шаг при
раннем возврате из Native посреди периода; RAM/команды/модели сравниваются
точно. Дополнительный CPU сценарий принудительно сохраняет прежнее
разрешение движения игрока и подтверждает, что при fade он не обновляется.

Строгий Source replay использует1163 exact frozen/current Source bodies,
Original20/30 и Native60/73/120/144/165/360, три сценария, реальные
constructors/parser/contact/SDK/retirement,96 периодов либо настоящий
handoff сцены. Проверены связи с218 врагами, игроком, оружием, уроном,
предметами, переходами, камерами, катсценами и игровым UI.
Сохранённые полные варианты имеют checksum бинарников/логов и доказанные
неизменённые ветви; cached player groups сохраняют фактическую execution
provenance. Они не выдаются за свежие запуски.

Каталог2532 описаний функций,2201 смысловых aliases и5765 scoped variable
names проверен: canonical C, preprocessing tokens, syntax и ABI сохранены.
Документы и старые адресные соответствия обновлены.32 dependency pins и
donor неизменны; прежние EXE, ROM, настройки, сейвы и DLL сохранены.

CPU модели/клипы/геометрия/ресурсы/звук синтетические; часть EV programs
синтетическая с настоящим parser. Достигнутые состояния перечислены.
Это не доказательство всех сюжетных ветвей и настоящих уровней. Контакт
игрока с движущейся платформой и внешний вид проверяет пользователь.
Игра/renderer/GPU локально не запускались. Замечания по визуальному
результату врагов043, включая идентификацию механического кролика,
не объявляются закрытыми без пользовательской проверки.

Выбранный блок1 реализован, CPU проверен и включён в Windows044.
Весь variable FPS ещё не завершён. Следующий блок выбирает пользователь:
4 — миниигры/специальные режимы;6 — итоговая интеграция всей игры.
Блоки2/3/5 реализованы ранее; новый блок автоматически не начинается.

# Блок1: объекты, платформы и оставшиеся NPC

Реализация блока1 и Source names/comments завершена и проверена единым connected pre-build29 checks. 289 типов /292 ROM-qualified variants: NPC, EV actors, disappearing/damage effects, платформы, двери, переключатели/ловушки и связанные механизмы. Common XYZ/model time и reviewed scalar/phase equations получают Native dt; Source counters, contacts, RNG, story, clip, spawn/retire выполняются в исходном порядке. Source-order parent paths и Door109 foreign-player increments защищены lifetime/epoch. Полный новый CPU proof29 checks PASS; Windows044 сохранён.

В реализации зарегистрировано 289 из 289 дополнительных/повторно проверяемых типов (292 варианта с точной ROM identity). Генерация startup-scene-block1-v4 и проверка имён выполнены. Connected CPU pre-build29 checks PASS; выявленные ошибки исправлены с повтором их областей и изменённых зависимостей. Windows044 сохранён. PASS043 относится к прежнему коду.

| Тип | ROM descriptor label | Update | Состояние |
|---:|---|---|---|
| 29 | ChrNamePepinStayMan0 | 0x8022de28 | Реализовано; connected CPU PASS044 |
| 30 | ChrNamePepinStayWoman0 | 0x8022de28 | Реализовано; connected CPU PASS044 |
| 31 | ChrNamePepinWalkMan0 | 0x8022e7b4 | Реализовано; connected CPU PASS044 |
| 32 | ChrNamePepinWalkWoman0 | 0x8022e7b4 | Реализовано; connected CPU PASS044 |
| 33 | ChrNamePepinSlimeMan0 | 0x8022e9a8 | Реализовано; connected CPU PASS044 |
| 34 | ChrNamePepinSlimeWoman0 | 0x8022e9a8 | Реализовано; connected CPU PASS044 |
| 35 | ChrNameMinikkuStayMan0 | 0x8022f2c8 | Реализовано; connected CPU PASS044 |
| 36 | ChrNameMinikkuStayWoman0 | 0x8022f2c8 | Реализовано; connected CPU PASS044 |
| 37 | ChrNameMinikkuWalkMan0 | 0x8022fc90 | Реализовано; connected CPU PASS044 |
| 38 | ChrNameMinikkuWalkWoman0 | 0x8022fc90 | Реализовано; connected CPU PASS044 |
| 39 | ChrNamePopikaStayMan0 | 0x80230494 | Реализовано; connected CPU PASS044 |
| 40 | ChrNamePopikaStayWoman0 | 0x80230494 | Реализовано; connected CPU PASS044 |
| 41 | ChrNamePopikaWalkMan0 | 0x80230b48 | Реализовано; connected CPU PASS044 |
| 42 | ChrNamePopikaSlimeMan0 | 0x80230d5c | Реализовано; connected CPU PASS044 |
| 43 | ChrNamePopikaSlimeWoman0 | 0x80230d5c | Реализовано; connected CPU PASS044 |
| 44 | ChrNamePopikaFlyMan0 | 0x80230f48 | Реализовано; connected CPU PASS044 |
| 45 | ChrNamePopikaFlyWoman0 | 0x80230f48 | Реализовано; connected CPU PASS044 |
| 46 | ChrNamePopikaFuyuu0 | 0x80231a90 | Реализовано; connected CPU PASS044 |
| 47 | ChrNameDakutoStayMan0 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 48 | ChrNameDakutoStayMan1 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 49 | ChrNameDakutoStayWoman0 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 50 | ChrNameDakutoStayWoman1 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 51 | ChrNameDakutoStayBoy0 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 52 | ChrNameDakutoStayBoy1 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 53 | ChrNameDakutoStayBoy2 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 54 | ChrNameDakutoStayGirl0 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 55 | ChrNameDakutoStayGirl1 | 0x802311f8 | Реализовано; connected CPU PASS044 |
| 56 | ChrNameDakutoWalkMan0 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 57 | ChrNameDakutoWalkMan1 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 58 | ChrNameDakutoWalkWoman0 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 59 | ChrNameDakutoWalkWoman1 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 60 | ChrNameDakutoWalkBoy0 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 61 | ChrNameDakutoWalkBoy1 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 62 | ChrNameDakutoWalkBoy2 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 63 | ChrNameDakutoWalkGirl0 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 64 | ChrNameDakutoWalkGirl1 | 0x802320d8 | Реализовано; connected CPU PASS044 |
| 65 | ChrNameDakutoKeeperMan0 | 0x80232910 | Реализовано; connected CPU PASS044 |
| 66 | ChrNameDoragonia1 | 0x8023e7d0 | Реализовано; connected CPU PASS044 |
| 67 | ChrNameDoragonia2 | 0x8023ec14 | Реализовано; connected CPU PASS044 |
| 68 | ChrNameDoragoniaStay0 | 0x80232238 | Реализовано; connected CPU PASS044 |
| 69 | ChrNameDoragoniaGuard | 0x80232690 | Реализовано; connected CPU PASS044 |
| 70 | ChrNameDoragoniaBoss | 0x80232c04 | Реализовано; connected CPU PASS044 |
| 78 | ChrNameBroboNB0 | 0x802401a0 | Реализовано; connected CPU PASS044 |
| 79 | ChrNameSprit | 0x8023dce4 | Реализовано; connected CPU PASS044 |
| 81 | ChrNameSekiban | 0x8023f730 | Реализовано; connected CPU PASS044 |
| 109 | ChrNameDokodemodoa | 0x801ea86c | Реализовано; connected CPU PASS044 |
| 110 | ChrNameDokodemodoalight | 0x801ea4d4 | Реализовано; connected CPU PASS044 |
| 132 | ChrNameTestchr1 | 0x801e9ecc | Реализовано; connected CPU PASS044 |
| 133 | ChrNameTestchr2 | 0x801e98e0 | Реализовано; connected CPU PASS044 |
| 134 | ChrNameTestchr3 | 0x801e9664 | Реализовано; connected CPU PASS044 |
| 135 | ChrNameTestchr4 | 0x801e9428 | Реализовано; connected CPU PASS044 |
| 136 | ChrNameEvDoraemon | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 137 | ChrNameEvNobita | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 138 | ChrNameEvSizuka | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 139 | ChrNameEvSuneo | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 140 | ChrNameEvJaian | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 141 | ChrNameEvDorami | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 142 | ChrNameEvDoraemon0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 143 | ChrNameEvNobita0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 144 | ChrNameEvSizuka0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 145 | ChrNameEvSuneo0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 146 | ChrNameEvJaian0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 147 | ChrNameEvDorami0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 148 | ChrNameEvReina0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 149 | ChrNameEvNobiru0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 150 | ChrNameEvNobitaisu | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 151 | ChrNameEvNobitahikidashi | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 152 | ChrNameEvPepinnomachi | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 153 | ChrNameEvDoragonianomachi | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 154 | ChrNameEvPopikanomachi | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 155 | ChrNameEvNobitanomachi0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 156 | ChrNameEvNobitanomachi1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 157 | ChrNameEvTachiagaruhikari | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 158 | ChrNameEvHoshichikyuu | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 159 | ChrNameEvHoshidakuto | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 160 | ChrNameEvHoshiryokuka | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 161 | ChrNameEvInseki | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 162 | ChrNameEvDokan | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 163 | ChrNameEvBook | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 164 | ChrNameEvBookdora | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 165 | ChrNameEvInsekiawase | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 166 | ChrNameEvInsekisizuka | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 167 | ChrNameEvInsekisuneo | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 168 | ChrNameEvInsekijaian | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 169 | ChrNameEvInazuma | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 170 | ChrNameEvTimemachine | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 171 | ChrNameEvLeaf | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 172 | ChrNameEvSuraimu | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 173 | ChrNameEvSizukahuro | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 174 | ChrNameEvBs01kaidan | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 175 | ChrNameEvJairocapsel0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 176 | ChrNameEvJairocapsel1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 177 | ChrNameEvUchuubort | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 178 | ChrNameEvBagururobo | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 179 | ChrNameEvBs03stage | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 180 | ChrNameEvBs03plate | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 181 | ChrNameEvBs03speeker | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 182 | ChrNameEvBs03body | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 183 | ChrNameEvWarphole | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 184 | ChrNameEvBs04body | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 185 | ChrNameEvInsekiawasenohikari | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 186 | ChrNameEvKyuushutsuhikari | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 187 | ChrNameEvMajin | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 188 | ChrNameEvGarokkutower0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 189 | ChrNameEvGarokkutower1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 190 | ChrNameEvGarokkuhoutou | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 191 | ChrNameEvEndbg00 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 192 | ChrNameEvEndbg01 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 193 | ChrNameEvEndbg02 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 194 | ChrNameEvEndbg03 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 195 | ChrNameEvEndbg04 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 196 | ChrNameEvEndbg05 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 197 | ChrNameEvEndbg06 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 198 | ChrNameEvEndbg07 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 199 | ChrNameEvEndbg08 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 200 | ChrNameEvEndbg09 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 201 | ChrNameEvEndbg10 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 202 | ChrNameEvEndbg11 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 203 | ChrNameEvEndbg12 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 204 | ChrNameEvSentoutei | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 205 | ChrNameEvSentouteitama | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 206 | ChrNameEvBakuhatsu0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 207 | ChrNameEvBakuhatsu1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 208 | ChrNameEvBs05body | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 209 | ChrNameEvBs02body | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 210 | ChrNameEvBs02leg | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 211 | ChrNameEvBs02head | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 212 | ChrNameEvBs02Eleveter | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 213 | ChrNameEvUchuubortdoor | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 214 | ChrNameEvMajinwave | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 215 | ChrNameEvBagurustation | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 216 | ChrNameEvMajinlaser | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 217 | ChrNameEvBs04Timehole | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 218 | ChrNameEvBaguruhelmet | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 219 | ChrNameEvBs07bug | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 220 | ChrNameEvTimemachineback | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 221 | ChrNameEvWatercleanship | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 222 | ChrNameEvBs03Crash | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 223 | ChrNameEvPopikaStayMan0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 224 | ChrNameEvPopikaFlyWoman0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 225 | ChrNameEvDoragoniaStay0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 226 | ChrNameEvMinikkuStayMan0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 227 | ChrNameEvMinikkuStayWoman0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 228 | ChrNameEvPepinStayMan0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 229 | ChrNameEvPepinStayWoman0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 230 | ChrNameEvDoramiTimemachine | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 231 | ChrNameEvMajinsunakemuri | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 232 | ChrNameEvMajinbubble | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 233 | ChrNameEvCopter0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 234 | ChrNameEvCopter1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 235 | ChrNameEvCopter2 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 236 | ChrNameEvCopter5 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 237 | ChrNameEvHanazono0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 238 | ChrNameEvHanazono1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 239 | ChrNameEvHanazono2 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 240 | ChrNameEvHanazono3 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 241 | ChrNameEvSmoke0 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 242 | ChrNameEvSmoke1 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 243 | ChrNameEvSmoke2 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 244 | ChrNameEvSmoke3 | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 245 | ChrNameEvMaskslime | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 246 | ChrNameEvBs04mini | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 247 | ChrNameEvCard | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 248 | ChrNameEvDemostar | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 249 | ChrNameShoumetsu0 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 250 | ChrNameShoumetsu1 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 251 | ChrNameShoumetsu2 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 252 | ChrNameShoumetsu3 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 253 | ChrNameShoumetsu4 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 254 | ChrNameShoumetsu5 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 255 | ChrNameShoumetsu6 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 256 | ChrNameShoumetsu7 | 0x801e52d4 | Реализовано; connected CPU PASS044 |
| 260 | ChrNameEfdamagefire | 0x801e58e0 | Реализовано; connected CPU PASS044 |
| 261 | ChrNameEfdamagecold | 0x801e5a94 | Реализовано; connected CPU PASS044 |
| 262 | ChrNameEfdamagekanden | 0x801e5c2c | Реализовано; connected CPU PASS044 |
| 263 | ChrNameHuyuplate | 0x801ea000 | Реализовано; connected CPU PASS044 |
| 479 | ChrNameM023_PaintBlock | 0x8023ef1c | Реализовано; connected CPU PASS044 |
| 480 | ChrNameM024_BridgePiece | 0x8023f268 | Реализовано; connected CPU PASS044 |
| 481 | ChrNameExitMark | 0x801eb440 | Реализовано; connected CPU PASS044 |
| 482 | ChrNameExitMarkSmall | 0x801eb440 | Реализовано; connected CPU PASS044 |
| 483 | ChrNameWarpHole1 | 0x801eac9c | Реализовано; connected CPU PASS044 |
| 484 | ChrNameWarpHole2 | 0x801ead74 | Реализовано; connected CPU PASS044 |
| 485 | ChrNameTreasureBox1 | 0x801eb7bc | Реализовано; connected CPU PASS044 |
| 486 | ChrNameTreasureBox2 | 0x801eb7bc | Реализовано; connected CPU PASS044 |
| 487 | ChrNameTreasureBox3 | 0x801eb7bc | Реализовано; connected CPU PASS044 |
| 488 | ChrNameSoundSwitch | 0x8023dbd0 | Реализовано; connected CPU PASS044 |
| 489 | ChrNameDomeroSwitch | 0x8023e798 | Реализовано; connected CPU PASS044 |
| 490 | ChrNameMariruPlate | 0x8023e6c0 | Реализовано; connected CPU PASS044 |
| 491 | ChrNameMariruSwitch | 0x8023e94c | Реализовано; connected CPU PASS044 |
| 492 | ChrNameDancingFlower | 0x8023ea4c | Реализовано; connected CPU PASS044 |
| 493 | ChrNameM002_Door00 | 0x8022ede8 | Реализовано; connected CPU PASS044 |
| 494 | ChrNameM002_Door01 | 0x8022ef9c | Реализовано; connected CPU PASS044 |
| 495 | ChrNameM003_Door00 | 0x8023f93c | Реализовано; connected CPU PASS044 |
| 496 | ChrNameM004_Door00 | 0x8023e0d8 | Реализовано; connected CPU PASS044 |
| 497 | ChrNameM006_Door01 | 0x8023df60 | Реализовано; connected CPU PASS044 |
| 498 | ChrNameM006_Door03 | 0x8023e330 | Реализовано; connected CPU PASS044 |
| 499 | ChrNameM007_Door01 | 0x8023e9a8 | Реализовано; connected CPU PASS044 |
| 500 | ChrNameM007_Shield | 0x8023eb18 | Реализовано; connected CPU PASS044 |
| 501 | ChrNameM009_Door00 | 0x8023da00 | Реализовано; connected CPU PASS044 |
| 502 | ChrNameM009_Door01 | 0x8023dd4c | Реализовано; connected CPU PASS044 |
| 503 | ChrNameM009_Door02 | 0x8023e098 | Реализовано; connected CPU PASS044 |
| 504 | ChrNameM009_Door03 | 0x8023e3f4 | Реализовано; connected CPU PASS044 |
| 505 | ChrNameM010_Fusya | 0x8023d840 | Реализовано; connected CPU PASS044 |
| 506 | ChrNameM010_Light | 0x8023d8e4 | Реализовано; connected CPU PASS044 |
| 507 | ChrNameM010_Monorail | 0x8023dcd8 | Реализовано; connected CPU PASS044 |
| 508 | ChrNameM015_Door00 | 0x8023e7a4 | Реализовано; connected CPU PASS044 |
| 509 | ChrNameM015_Door01 | 0x8023eb54 | Реализовано; connected CPU PASS044 |
| 510 | ChrNameM015_Door02 | 0x8023eea0 | Реализовано; connected CPU PASS044 |
| 511 | ChrNameM016_Dai | 0x8022f7f0 | Реализовано; connected CPU PASS044 |
| 512 | ChrNameM020_IgaHead | 0x8022df54 | Реализовано; connected CPU PASS044 |
| 513 | ChrNameM025_Crystal | 0x8023de34 | Реализовано; connected CPU PASS044 |
| 514 | ChrNameSoundDoor | 0x8023e4c0 | Реализовано; connected CPU PASS044 |
| 515 | ChrNameSoundLightBlue | 0x8023e334 | Реализовано; connected CPU PASS044 |
| 516 | ChrNameSoundLightRed | 0x8023e334 | Реализовано; connected CPU PASS044 |
| 517 | ChrNameSoundLightYellow | 0x8023e334 | Реализовано; connected CPU PASS044 |
| 518 | ChrNameContainer | 0x801ea470 | Реализовано; connected CPU PASS044 |
| 519 | ChrNameSpring1 | 0x8023d830 | Реализовано; connected CPU PASS044 |
| 520 | ChrNameKumo | 0x8023dea4 | Реализовано; connected CPU PASS044 |
| 521 | ChrNameTaiyo | 0x8023d768 | Реализовано; connected CPU PASS044 |
| 522 | ChrNameJackKuki | 0x8023d6e0 | Реализовано; connected CPU PASS044 |
| 523 | ChrNameJackLeafR | 0x8023d7d0 | Реализовано; connected CPU PASS044 |
| 524 | ChrNameJackTuchi | 0x8023db44 | Реализовано; connected CPU PASS044 |
| 525 | ChrNameKaitenDai | 0x8022eb44 | Реализовано; connected CPU PASS044 |
| 526 | ChrNameKaitenPole | 0x8022ec00 | Реализовано; connected CPU PASS044 |
| 527 | ChrNameKaitenButton | 0x8022ec18 | Реализовано; connected CPU PASS044 |
| 528 | ChrNameKaitenGlass | 0x8022ed9c | Реализовано; connected CPU PASS044 |
| 529 | ChrNameControlStone | 0x8022f15c | Реализовано; connected CPU PASS044 |
| 530 | ChrNameMetarudoDoor1 | 0x8022f680 | Реализовано; connected CPU PASS044 |
| 531 | ChrNameMetarudoDoor2 | 0x8022f680 | Реализовано; connected CPU PASS044 |
| 532 | ChrNameMetarudoPanel1 | 0x8022f944 | Реализовано; connected CPU PASS044 |
| 533 | ChrNameMetarudoPanel2 | 0x8022f944 | Реализовано; connected CPU PASS044 |
| 534 | ChrNameTowerFlash | 0x8022f9ec | Реализовано; connected CPU PASS044 |
| 535 | ChrNameSwitchBase | 0x801eae54 | Реализовано; connected CPU PASS044 |
| 536 | ChrNameSwitchLight | 0x801eae6c | Реализовано; connected CPU PASS044 |
| 537 | ChrNameStoneArm | 0x8023e700 | Реализовано; connected CPU PASS044 |
| 538 | ChrNameTrapDai | 0x8022fa84 | Реализовано; connected CPU PASS044 |
| 539 | ChrNameTrapFire | 0x8022fce8 | Реализовано; connected CPU PASS044 |
| 540 | ChrNameSpeaker | 0x8023e96c | Реализовано; connected CPU PASS044 |
| 541 | ChrNameWaterDrop | 0x8023d844 | Реализовано; connected CPU PASS044 |
| 542 | ChrNameM022_Dai | 0x8023ebd8 | Реализовано; connected CPU PASS044 |
| 543 | ChrNameSaku | 0x8023db14 | Реализовано; connected CPU PASS044 |
| 544 | ChrNameYajirusi | 0x801eaf68 | Реализовано; connected CPU PASS044 |
| 545 | ChrNameBook1 | 0x8023d768 | Реализовано; connected CPU PASS044 |
| 546 | ChrNameBook2 | 0x8023d854 | Реализовано; connected CPU PASS044 |
| 547 | ChrNameM051_Sibuki | 0x801eb0ec | Реализовано; connected CPU PASS044 |
| 548 | ChrNameIcePlateBig | 0x8023dd98 | Реализовано; connected CPU PASS044 |
| 549 | ChrNameIcePlateSmall | 0x8023dea0 | Реализовано; connected CPU PASS044 |
| 550 | ChrNameM101_Plate | 0x8023d85c | Реализовано; connected CPU PASS044 |
| 551 | ChrNameM102_Plate | 0x8023d938 | Реализовано; connected CPU PASS044 |
| 552 | ChrNameMiireSwitch | 0x8023dbbc | Реализовано; connected CPU PASS044 |
| 553 | ChrNameM103_Plate | 0x8023dccc | Реализовано; connected CPU PASS044 |
| 554 | ChrNameM104_Plate | 0x8023dccc | Реализовано; connected CPU PASS044 |
| 555 | ChrNameMiireControl | 0x8023df70 | Реализовано; connected CPU PASS044 |
| 556 | ChrNameSnow | 0x8023dff4 | Реализовано; connected CPU PASS044 |
| 557 | ChrNameKogooruSwitch | 0x8023e4a0 | Реализовано; connected CPU PASS044 |
| 558 | ChrNameKogooruPlate | 0x8023e7a4 | Реализовано; connected CPU PASS044 |
| 559 | ChrNameRatenaPlate | 0x8023eda4 | Реализовано; connected CPU PASS044 |
| 560 | ChrNameRatenaSwitch1 | 0x8023f1c8 | Реализовано; connected CPU PASS044 |
| 561 | ChrNameRatenaSwitch2 | 0x8023f1c8 | Реализовано; connected CPU PASS044 |
| 562 | ChrNameWheel1 | 0x8023ecd8 | Реализовано; connected CPU PASS044 |
| 563 | ChrNameWheel2 | 0x8023ecd8 | Реализовано; connected CPU PASS044 |
| 564 | ChrNameGondola | 0x8023ed3c | Реализовано; connected CPU PASS044 |
| 565 | ChrNameM087_Haguruma | 0x8023ddf4 | Реализовано; connected CPU PASS044 |
| 566 | ChrNameM088_Plate | 0x8023dec4 | Реализовано; connected CPU PASS044 |
| 567 | ChrNameSoundTama | 0x8023f5e4 | Реализовано; connected CPU PASS044 |
| 568 | ChrNameM008_Control | 0x8023f138 | Реализовано; connected CPU PASS044 |
| 569 | ChrNameM107_Plate1 | 0x8023e178 | Реализовано; connected CPU PASS044 |
| 570 | ChrNameM107_Plate2 | 0x8023e178 | Реализовано; connected CPU PASS044 |
| 571 | ChrNameM107_Plate3 | 0x8023e254 | Реализовано; connected CPU PASS044 |
| 572 | ChrNameBaguruSwitch1 | 0x8023e514 | Реализовано; connected CPU PASS044 |
| 573 | ChrNameBaguruSwitch2 | 0x8023e514 | Реализовано; connected CPU PASS044 |
| 574 | ChrNameGondolaBridge | 0x8023f214 | Реализовано; connected CPU PASS044 |
| 575 | ChrNameDezuSwitch | 0x8023f3d0 | Реализовано; connected CPU PASS044 |
| 576 | ChrNameDezuPlate | 0x8023f59c | Реализовано; connected CPU PASS044 |
| 577 | ChrNameMetarudoShildCtrl | 0x8023e1e8 | Реализовано; connected CPU PASS044 |
| 578 | ChrNameM003_Door01 | 0x8023fb30 | Реализовано; connected CPU PASS044 |
| 579 | ChrNameM011_Door00 | 0x8022f5bc | Реализовано; connected CPU PASS044 |
| 580 | ChrNameM014_Door00 | 0x80231178 | Реализовано; connected CPU PASS044 |
| 581 | ChrNameM014_Door01 | 0x80231320 | Реализовано; connected CPU PASS044 |
| 582 | ChrNameMajinLight | 0x8023f61c | Реализовано; connected CPU PASS044 |
| 583 | ChrNameMajinEffect | 0x8023f6ac | Реализовано; connected CPU PASS044 |
| 584 | ChrNameM106_Door | 0x802313e8 | Реализовано; connected CPU PASS044 |
| 585 | ChrNameDoramiTimeMachine | 0x8023e2d4 | Реализовано; connected CPU PASS044 |
| 586 | ChrNameDoramiGurun | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 587 | ChrNameNobiheyaSlime | 0x8023dc78 | Реализовано; connected CPU PASS044 |
| 588 | ChrNameGoddess | 0x8023fc40 | Реализовано; connected CPU PASS044 |
| 589 | ChrNameStoneBase | 0x801e5d84 | Реализовано; connected CPU PASS044 |
| 590 | ChrNameNobiheyaDrawer | 0x8023e0b8 | Реализовано; connected CPU PASS044 |
| 591 | ChrNameEndFlag | 0x801e5d84 | Реализовано; connected CPU PASS044 |
