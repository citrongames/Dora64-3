## 2026-10-10: test100 accepted; widescreen commit/push authorized

User reported that everything now appears normal after test100 and explicitly
requested commit/push to existing main. This accepts the widescreen implementation
and corrections through100. Historical pending/no-push entries below are
superseded for this completed work only. Preserve accepted camera095 and all
earlier fixes. User feedback is gameplay evidence, not exhaustive scene coverage.
No agent tests, CPU checks, game/emulator/renderer/GPU runs. No additional build:
test100 remains the delivered executable. See docs/widescreen.md.

## 2026-10-10: Windows test-100 compiled/packaged; widescreen user check pending

Incremental MSVC compile/link exit0. No game/tests/CPU/GPU execution. Preserve
accepted095 camera and previous fixes. Whole puzzle compositions now share the original centered canvas;
blue fills only outside margins. User playcheck pending; no commit/push until authorization. See docs/widescreen.md.

## 2026-10-10: Windows test-099 compiled/packaged; widescreen user check pending

Incremental MSVC compile/link exit0. No game/tests/CPU/GPU execution. Preserve
accepted095 camera and previous fixes. Start lights, puzzle blue margins and grenade portraits/HP
need user playcheck; no commit/push until authorization. See docs/widescreen.md.

## 2026-10-10: Windows test-098 compiled/packaged; widescreen user check pending

Incremental MSVC compile/link exit0. No game/tests/CPU/GPU execution. Preserve
accepted095 camera and previous fixes. Widescreen call-scope crash correction after user097 report
need user playcheck; no commit/push until authorization. See docs/widescreen.md.

## 2026-10-10: Windows test-097 compiled/packaged; widescreen user check pending

Incremental MSVC compile/link exit0. No game/tests/CPU/GPU execution. Preserve
accepted095 camera and previous fixes. Widescreen fixes after user096 report
need user playcheck; no commit/push until authorization. See docs/widescreen.md.

## 2026-10-10: Windows test-096 compiled/packaged; widescreen user check pending

Incremental MSVC compile/link exit0. No game/tests/CPU/GPU execution. Preserve
accepted095 camera and previous fixes. New widescreen settings/layout/readback
need user playcheck; no commit/push until authorization. See docs/widescreen.md.

## 2026-10-10: widescreen implementation; build authorized, user playcheck pending

User approved implementation and one incremental Windows build at completion.
Commit/push only after user check. No tests/CPU checks/game/emulator/renderer/GPU.
Preserve accepted095 camera and earlier fixes. See docs/widescreen.md. Donor and
sources/ stay read-only. This task changes settings, explicit ROM layout hooks,
isolated pinned RT64 adapters and wide deferred-readback coordinates.

## 2026-10-10: test095 accepted; camera commit/push authorized

User reported test095 appears to work and explicitly requested commit/push.
This accepts the Modern camera implementation and the test094 zone-reentry crash
fix for this task. It is user playcheck evidence, not exhaustive scene coverage.
Preserve all accepted changes through093. Historical pending/no-push entries
below are superseded for the completed camera work only. No agent tests, CPU
checks, game/emulator/renderer/GPU were run. No additional build for this
documentation/commit step; test095 remains the delivered executable.

## 2026-10-10: Windows test-095 compiled/packaged for user camera check

Incremental Windows compile/link exit0; no application/test/CPU/GPU execution.
The test094 zone-reentry crash fix awaits user gameplay verification. No commit/push yet. Existing
test093 and prior accepted fixes retained. Next action depends on user's test.

## 2026-10-10: standing authorization to build immediately after fixes

User explicitly instructed: after fixes, immediately build a Windows EXE.
This supersedes earlier requirements for another build command after each fix.
Use incremental compilation and next existing build number; no tests/CPU checks,
game/emulator/renderer/GPU. Preserve old EXE/ROM/settings/saves/shared DLLs.
Commit/push still require the user's gameplay verification. Current build includes
the test094 Source/Native camera handoff crash fix documented below.

## 2026-10-10: test094 zone-reentry crash diagnosed and source fix pending build

User reported a crash after reentering zones. Read-only analysis of local dump
Dora64-3-test-094.exe.32628.dmp found fail-fast C0000409/7 and a C++ logic_error.
Saved stack addresses resolve against test094 PDB to SimulationSuspension
constructor, modern_camera.cpp update line171, camera::dispatch, and native
camera dispatch. Source review identifies the constructor's explicit exception
"Subsystem suspension outside simulation". Camera's Source shake path wrongly
constructed a Native suspension with no active slice after a zone handoff.
Fixed by suspending only when a slice exists; Original calls the same shake
handler directly. UI fallback remains geometry-only and does not advance shake
commands. No generation changes; no tests/game/CPU/GPU runs. User asked to check;
no new Windows EXE without a build command. No commit/push before user playcheck.

## 2026-10-10: Windows test-094 compiled/packaged for user camera check

Incremental Windows compile/link exit0; no application/test/CPU/GPU execution.
Modern camera awaits user gameplay verification. No commit/push yet. Existing
test093 and prior accepted fixes retained. Next action depends on user's test.

## 2026-10-10: Modern camera implementation; user playcheck pending

User approved implementation after ROM/source analysis and requested the build
at completion. Commit/push only AFTER the user's gameplay check. No agent tests,
CPU checks, game/emulator/renderer/GPU execution. Preserve accepted093 and091/092.
Modern camera follows Original Z permission: scene1 ordinary modes3/4/5 only,
with scene/script/menu/dialogue/room and player-recovery gates. Other owners keep
Original camera. Dora64 Game-tab options/defaults/order/input are ported; its
mode0/1 permission and game addresses must NOT be copied. Original R/C prefix is
isolated through a ROM-qualified generator hook. All ordinary roots pass the
existing dispatcher; disabled mode retains original equations. Native gameplay
uses slice time; UI-only camera frames use incremental elapsed time within the
Source period. Reset clears orbit/input. See docs/modern-camera.md.
Windows generation: startup-modern-camera-v2, clean diagnostics OFF. Incremental
Windows staging reuses its existing CPU cache path with generator-produced files;
previous differing generated files/receipt are preserved before replacement.

## 2026-10-09:093 flower flicker fix accepted; commit/push authorized

User confirmed test093 fixes the flower flicker and explicitly requested commit
and push. Preserve deferred workload texture retention and earlier accepted fixes.
Clean093 compile/package completed; visual confirmation is the user's playcheck.
No agent tests/CPU checks/game/emulator/renderer/GPU were run. Historical pending
validation/no-push notes below are superseded for this completed fix only.

## After092: user bisects flower flicker to077;093 texture lifetime correction

076 normal,077 first bad;085 flower disappears,092 can show black transparency.
Deferred fullSync evicts before display createGPUTiles marks sampled hashes used.
Hash-hit upload skips refresh, so a returning texture can be evicted in its own
workload.093 refreshes current valid non-copy callTiles under the eviction mutex,
with allocation-free list splice and original bounded policy. Eager behavior,
GPU locks, performance paths, texture/alpha/animation/gameplay unchanged.
See docs/texture-lifetime-after092.md. Static defect; visual result pending.
No tests/CPU/game/emulator/renderer/GPU; incremental clean build only. No Git push.

## 2026-10-09: fixes through092 accepted; commit/push authorized

User confirmed091 swimming camera fix, then confirmed092 removed the lava loop
flash and explicitly requested commit and push. Preserve camera ownership join,
extended sampled-texture/TLUT addressing and whole-cycle CI4 continuity.
Clean Windows092 compile succeeded; user playcheck succeeded. No agent tests,
CPU checks, game, emulator, renderer or GPU were run. Historical notes below
retain their original validation status; this acceptance supersedes pending
playcheck and no-push restrictions for this completed set only.

## After091: camera confirmed; lava loop seam follow-up092

User confirms swimming camera fixed; lava generally improved but one-frame flash
at loop seam remains. Preserve091 ownership/address fixes. Static material
analysis: pair-local CI4 partitions disagree with each other and with ROM holds.
092 uses one content-qualified pixel partition over each compatible opaque CI4
loop, including holds and phases0/1. Palettes share identical end/start values.
Equal-color holds bypass lerp arithmetic to avoid half-integer rounding wobble.
Original assets/timing and exact UV scroll path unchanged. Quantized Native
keyframes may differ slightly in color from ROM; visual check pending.
No tests/CPU checks/game/GPU. Clean incremental build; no commit/push.

## After090: user rejected both visual fixes;091 addresses missed paths

090 did not fix lava or underwater swimming camera by user observation.
Static091 findings: DBE5C bypasses BE78 ownership hook for swimming mode3,
leaving camera_allowed false after BB080. Hook moves to shared BE84 join.
Unconfirmed090 pitch-law change removed; Original camera equations retained.
Animated bitmap FD pointers wrongly stripped kseg0 after frame arena expansion;
use display_address for sampled texels and TLUT, retaining extended RAM tag.
Applies to CI4 and IA8 sampled materials. 090 stable palette grouping retained.
No tests/CPU checks/game/renderer/GPU. Compile/package only; visual check pending.
No commit/push of these fixes without direct user command.

## After089: new lava/swimming-camera changes, FPS investigation paused

Previous stage08d2760 committed/pushed to private main on explicit usercommand.
New separate task: unstable lava fade; vertical swimming camera at world1 lake.
090 keeps CI4 transition membership fixed using endpoint-color trajectories;
mode3 pitch follows current player pitch with current dt, while other camera
fields/counters keep cumulative Source anchors. No Source commands/AI/contact
cuts. Static defects found; exact visual effect awaits user playcheck.
See docs/native-lava-swimming-after089.md. No agent tests/CPU/game/renderer/GPU.
Build clean. Do not commit/push these new changes without another direct command.

## 2026-10-09: performance work accepted for now; commit/push authorized

User paused further FPS optimization: mountain is a heavy scene, city can reach360.
Explicitly authorized commit and push of the previous stage through clean089.
088 user estimate140->160 remains the last numerical comparison; no separate089
controlled FPS result. New lava fade and swimming vertical-camera bugs are a
separate uncommitted task, not covered by this commit/push authorization.
No agent tests/CPU checks/game/renderer/GPU; source/ROM analysis and builds only.

## After088: user140->160FPS;089 removes repeated viewport clipping calculation

User reports approximately140->160FPS after088; estimate, no new profile.
087 HLE outsidefullSync2.971ms;085 trianglebounds sampled0.520ms includes
observer overhead. Static088 MSVC object has4 lroundf calls per visible triangle
for invariant viewport rect.089 caches pure rect by all4 XYfloat bit patterns
and4 int16 clip ratios, compares every call; original arithmetic on miss.
No geometry/gameplay/time/culling omissions;080/084/086/088 retained,082inactive.
Actual089 FPS unmeasured. Clean build; no agent game/tests/CPU/GPU.
See docs/performance-viewport-after088.md. Git only on explicit user command.

## After087: measured upload serialization;088 overlaps CPU tiles with GPU copy

087 profile1791480023665:53samples/errors0/drop0. Late165 producer5.057ms,
HLE outside fullSync2.971ms; tiles0.743ms then upload GPU wait0.682ms/frame.
SP/DP delivery mean0.262/0.142ms; RAM wait negligible.088 starts existing GPU
geometry copy before independent CPU tile/TMEM processing, then retains same
fence/texture waits before publication. Exceptional path also waits beforethrow.
No gameplay/AI/time/contact/bank/completion changes. Actual FPS unmeasured;
remaining HLE/Native costs persist, no360claim.080/084/086 retained,082 inactive.
See docs/performance-upload-after087.md, research/test087-performance-analysis.json.
Clean088; no agent tests/game/CPU checks/renderer/GPU; Git only on usercommand.

## After086: approximately same, possible10FPS;087 delivery timeline

User086 reports little change, perhaps10FPS. Not confirmed improvement.
087 observes actual SP/DP enqueue/delivery/guest receive, audio tasks and waits.
Hot-function sampling disabled to reduce observer overhead; schema6/stride0.
Keep080/084/086;082 inactive. No new performance fix claimed; see
 docs/performance-after086.md. Diagnostic permission persists; no agent game,
tests/CPU checks/renderer/GPU. Final fixes clean; Git only on command.

## After085: ready-frame idle persists;086 per-slice scheduler service

085 profile1791478659712:51samples/errors0/drop0;30..39s producer duty65.48%,
ready+producer-idle mean2.142ms. Common/player catch-up chains overlap the gap;
BB080 update bypasses FrameBridge entirely.086 services existing pending events
at fully unwound simulation boundaries via timeline-only current predicate.
No elapsed/slice/AI/contact/clip changes; no early SP/DP or bank-release changes.
080/084 preserved,082 batching inactive. Not all idle proven recoverable; FPS
pending user check. Clean086, no agent tests/game/CPU checks/renderer/GPU.
See docs/performance-handoff-after085.md and research/test085-critical-path-analysis.json.
Commit/push only on command; all dirty work and previous builds preserved.

## After084: user confirms +20-30FPS, about150;085 post-fix geometry measurements

084 checkpoints improve FPS by user observation20-30, still about150. Preserve
084 and080;082 batching remains inactive. No post084 internal profile yet.
085 keeps schema4 shared timeline plus schema5 geometry parts, active only
inside already-selected1/64 vertex/triangle samples. No per-vertex timers.
Parent sampled durations now include nested observer overhead; analyze part
sums using parent observed/calls, not as independent exact total times.
See docs/performance-after084.md. Diagnostic permission persists; no agent
execution of game/tests/CPU/GPU. Final fixes clean; Git only on command.

## After083: ready frames wait before runtime submission;084 explicit scheduler handoff

Profile1791477249843 schema4:54samples, zero scope errors/dropped events.
30..39s1163 matched frames:producer duty68.0%, ready+producer-idle mean1.996ms,
576frames>0.5ms, frame-build-end to runtime-submit mean4.871ms (includes busy SP).
This is measured handoff delay, not proof all of it is recoverable.
084 delivers already-pending external events and runs existing guest-priority
scheduler before/after Native update and after graphics wake enqueue. Original
SP/DP completion, banks, queue retention, time and all gameplay stay unchanged.
082 batching still disabled;080 cache retained. Clean084; user FPS pending.
Details docs/performance-handoff-after083.md; no agent game/tests/CPU/GPU.
Commit/push only by explicit user command; all dirty work preserved.

## After082: no user-visible gain;083 observes the frame critical path

User:082 approximately same FPS. Disable only082 geometry batching invocation;
keep080 proven TMEM cache and all prior functional fixes. Do not present082 as
successful.083 adds shared-clock timeline of Native update/build, source common
and player costs, RT64 producer/fullSync/render, task submit/parse, wake delivery,
deadline lateness and bank completion. FrameBuild includes blocking enqueue.
Static SP completion occurs after send_dl/fullSync; this is not yet proof of
harmful serialization. Do not remove lifetime waits without evidence/ownership.
See docs/performance-critical-path-after082.md. Diagnostic permission persists;
no agent game/tests/CPU checks/GPU. Commit/push only after explicit user command.

## After081: memo cache confirmed;082 batch geometry bookkeeping

User080 reported about2x FPS.081 profile1791476020198 stable165:137.38FPS,
cache99.9889% hit; texture0.734ms/frame, HLE self2.870ms, producer5.085ms.
Vertex/triangle sampled costs ~1.064/1.178ms/frame, estimates with overhead.
082 batches per-command vertex array growth and per-triangle UV bounds updates;
keeps all geometry, math, draw order, AI/time/contact. Actual additional gain
not measured; no360 claim. Build clean, no tests/game/GPU agent execution.
See docs/performance-geometry-after081.md and research/test081-remaining-performance-analysis.json.
All dirty work preserved; commit/push only on user command after playcheck.

## После080: пользователь подтвердил примерно двукратный прирост;081 уточняет остаток

080 exact-input TMEM hash memoization дала по наблюдению пользователя примерно2x
FPS, но360 не достигнуты. Это пользовательская оценка, не новый внутренний профиль.
081 сохраняет080 и добавляет cache hit/miss/bypass, sampled Native object/model/enemy
stages. Sampled timers теперь wall-only: GetThreadTimes/QueryThreadCycleTime исключены
из мелких измерений, coarse thread CPU остаётся. Schema3; вложенные/параллельные
затраты не суммировать, выборочные оценки всё ещё имеют observer overhead.
081 диагностический по ранее данному разрешению, generic traces/profiler выключены.
Не объявлять нынешнюю причину без нового профиля. Нет запуска игры/тестов агентом.
Подробности docs/performance-remaining-after080.md. Commit/push по команде.

## После079: измеренный TMEM hash;080 exact-input memoization, чистый билд

Профиль1791474301346 (schema2,41sample,scope_errors0): stable16526samples,
2169frames,82.92FPS; texture stage3.982мс/frame. Sampled TMEMHasher среднее
6.262мкс; extrapolation~3.719мс/frame включает observer overhead, не складывать
с вложенным upload/прочими оценками. Источник:research/test079-graphics-detail-analysis.json.
TextureManager::uploadTexture всегда пересчитывает hash доhashSet lookup.
080 memoizes pure paletted v5 hash только по точному совпадению всех4096bytes
и9параметров; GPUcache/eviction/uploads/readback и gameplay unchanged.
Bounded2048entries/~8.2MiB per producer thread. Hit rate/newFPS ещё не измерены,
не обещать300/360. После правки clean080: GraphicsProfileOFF/DiagnosticsOFF,
CPU generation прежняяclean; tests/game/GPU агентом не запускать; Git по команде.
Подробности docs/performance-tmem-hash-after079.md.

## После профиля078: измерены HLE и texture processing;079 уточняет внутреннюю причину

Пользовательский запуск1791473499098:2D360.14FPS, intro116.96, тяжёлая165
86.91FPS. В165 deferred3149/3149, eager0:077 fast path действительно работает.
Producer8.31мс/frame; HLE self2.835, tiles3.950, producer GPU wait0.785.
Не суммировать nested/параллельные stages; thread CPU мелких областей грубый.
078 ещё не разделяет loadToTMEM/TextureManager hash; точную внутреннюю причину
не заявлять.079 добавляет только1/64 sampled detail этих операций и HLE
vertex/triangle/matrix/draw-state/flush, без изменений алгоритмов. Диагностика
разрешена пользователем для расследования; tests/game/GPU агенту запрещены.
После измеренного исправления чистый билд; commit/push только по команде.

## После077: точечный графический профиль078 разрешён пользователем

077 не устранила просадку: Native5x/360 около120FPS, гора до70; начальная
катсцена около120; чистые2D экраны дают360. По наблюдению CPU/GPU около10%.
Это приблизительные наблюдения, не новые внутренние измерения. Причина
остаточной просадки НЕ установлена;077 исправила eligibility, не доказалаFPS.
Пользователь прямо разрешил точечный диагностический билд и пояснил: прежний
запрет профиля был для сравнения чистой скорости, для расследования можно.
078 только наблюдение, gameplay/077 fallback/barriers не меняются. Общие
DIAGNOSTICS/PROFILE и generated traces выключены; отдельный GRAPHICS_PROFILE
с coarse stage timers/counters, один graphics-profile.jsonl/сек. После
измеренного исправления вернуть чистую сборку. Агент игру/тесты не запускает.
Подробности docs/performance-graphics-profile-after077.md. Git только по команде.

## После076: исправлены два препятствия deferred readback

Статически подтверждено: Original задаёт Z00000400, а076 ожидала0038F800;
кроме того,076 отвергала штатную SetColorImage(Z)+FillRect очистку. Общий
planner теперь принимает реальный адрес и RT64 clearDepthOnly, сохраняя
результат какDepth. Другие fallback/barriers/AI/time/contacts не менялись.
Детали docs/performance-depth-readback-after076.md. Новый FPS не измерен,
300/360 не обещаны; clean077 по команде, без тестов/запуска и commit/push.

## После076: пользователь не заметил улучшения; причина остаточной просадки не установлена

Пользователь после проверки076: «ну примерно тоже самое». Предыдущая оценка:
360FPS только в2D меню/маленьких комнатах; обычный мир около80–110FPS,
при дальности1x максимум порядка200+. Новые точные замеры076 не предоставлены.
Отдельный RAM-render вfullSync найден по исходникам, но его устранение в076
не дало заметного пользовательского эффекта. Не считать это доказанной причиной
остаточной просадки и не утверждать, что fast path реально работал в проблемной
сцене:076 без профиля/логов, фактическая eligibility/fallback частота не измерена.
Следующая задача передаётся Астре: независимо локализовать оставшееся узкое место,
проверить применение076 и изучить весь критический graphics/Native путь.
Новых исправлений или сборки при передаче не сделано. Commit/push запрещены
до пользовательской проверки и прямой команды. Все незакоммиченные изменения
main сохранить; не применять reset/clean и упрощения игровой логики.

##076: deferred Native framebuffer RAM; clean build

075 user: world80–110FPS, drawdistance1x up to200+,360 onlysimple scenes.
Найдена интеграционная причина: frontend default renderToRAM=true заставляет
State fullSync отдельно рисовать native scene для RAM перед вторым display
рендером.076 переносит RAM копию eligible Native gameplay постоянных color/Z
буферов к CPU copy/Native handoff/config/fallback; Original/raw/unsupported
пути сохраняют eager handling. Есть масштабированные native pixel coordinates
дляAuto/2x/4x. Не выключать CPU copy и не упрощатьAI/time/contact.
Подробности docs/performance-framebuffer-readback.md. Legacy eager shader flag0
неизменён; pinned libraries read-only/build-local adapters.076 clean, безлогов,
profiling/tests/CPU checks/game/GPU agent execution. Git после команды.

##075: чистая скорость без диагностической нагрузки

Прямая инструкция пользователя: следующий билд без проверок и логов.
Windows075 использует --no-diagnostics CPU generation, profiling=false,
DORA3_DIAGNOSTICS=OFF: нет generic trace entries, timers/atomic profiler,
per-task/audio/input logs, ресурсных снимков и исследовательских byte checks.
Сохранены12 ROM-qualified functional entries и реальная overlay/queue/lifetime
логика. Не отключать игровые условия/AI/contact/time радиFPS.075 сравнивает
нагрузку диагностики с074; ещё не доказывает устранения расходов graphics producer.
Подробнее docs/performance-clean-build.md. Старые файлы/сейвы сохранять;
нет tests/CPU checks/game/GPU запуска агентом; commit/push после команды.

##074: пользователь подтвердил60; исследованиеManual360

После исправления entry registry пользователь получил60FPS. Новый журнал
1791464902194 подтверждает accepted360 и355–360FPS в переходах,121 при входе
в169; универсального100FPS cap нет. На5x тяжёлые сцены165/169/150 дают примерно
61/95/97FPS. Следующая локализация — producer RT64Parse (CPU6–8мс/frame,
GPU~0.3мс), не уменьшениеAI или границвремени. Build RelWithDebInfo /O2/NDEBUG;
RT64 verbose logging disabled, собственные task/audio logs и profiler enabled.
Доля logging overhead не измерена отдельно; не объявлять его всей причиной.
Подробности docs/performance-after074.md. Только статический анализ/сборка,
игру/тесты агентом не запускать; Git после команды пользователя.

## После073: измеренный служебный entry registry;074

Профиль073 active5x46–48FPS, pause60. Около2.8млн trace calls/сек;
каждый повторно сканировал49 immutable overlay records и91 dependency records
при совпадении. Trace sampled duration раньше нельзя брать без scale1/64;
оценка порядка383мс/сек wall, не точная CPU attribution.074 исправляет
immutable lookup/first-entry registry, сохраняя actual atomic loaded/generation
проверки на каждом вызове и все special hooks. Тайминг/AI/contact/input не менять.
Подробности docs/performance-entry-registry.md; игра/тесты агентом запрещены,
Git после пользовательской проверки. Не применять speculative input coalescing.

## После072: исследовать excessive full-pool subdivision

Пользователь требует хороший быстрый результат по теме; просадку ниже60
не объяснять настройкой лимита.072 на5x около32FPS, Native common516 calls/сек,
Original20 calls/сек.073 пишет boundary reasons/slice histogram и thread CPU
durations; time/AI/contact/birth/clip rules не менять без доказанной причины.
Для следующего замера достаточно5x+пауза; не гонять снова все значения1..5.
Тесты/CPU checks/игра/renderer/GPU агентом запрещены. Git после user проверки.

## После профиля071: измеренный Native dispatcher и identity lookup

071 показал миллионы повторных служебных preview/eligibility/identity calls;
на5x dispatcher self примерно500мс/сек по выборке.072 исправляет только
маршрутизацию подтверждённого E8E7C и порядок site guards, а также поиск
immutable definitions; cleanup/AI/contacts/elapsed time не сокращать.
Профиль сохраняется, улучшение072 проверяет пользователь. Подробности:
docs/performance-native-dispatch.md. Git только после проверки и команды.

## Текущее состояние профилирования после070

070:3x улучшилось до58–60FPS,4x/5x остаются просадки. Профиль локализует
остаток в common pool self; отдельная причина внутри ещё не измерена.
071 добавляет только sampled scopes/observed_calls с сохранением070 логики.
Следующее исправление выбирать по071 измерениям, без упрощений AI/contacts
или ограничения elapsed time. Игру/профиль запускает только пользователь.

## Диагностика производительности: сначала доказанная причина (2026-10-08)

Прямая инструкция пользователя после068: не вносить оптимизации, упрощения
логики или предположительные исправления FPS без установления реальной причины.
Сначала изучить код и измерить именно проблемный сценарий. Пользователь разрешил
профилировочный Windows build для определения того, что съедает FPS. В нём
сохранять игровую логику/дальность/лимиты068, добавлять только наблюдение и журнал.
Неподтверждённые оптимизации текущего этапа отменены. После пользовательского
профиля назвать измеренное узкое место, затем исправлять его целевым изменением.
Без запусков игры/renderer/GPU/CPU checks агентом; профиль собирает пользователь.
Commit/push по-прежнему после проверки и отдельной команды.

## Действующее правило сборки: после завершённых изменений кода (2026-10-08)

Прямая инструкция пользователя: «после того как ты сделал все изменения в коде,
делай билд». После завершения очередного согласованного набора изменений кода
сразу собирать и упаковывать новый Windows EXE в D:/Games/DoraRecomp3/testbuild;
не ждать отдельной команды «билд» или «нет». Не собирать после каждого промежуточного
файла; завершить изменения текущей задачи, затем сделать один билд. Это заменяет
прежнее требование отдельного разрешения на каждую следующую сборку.
Документы/receipts после сборки не требуют нового EXE. Тесты и запуск игры агентом
по-прежнему запрещены. Commit/push только после пользовательской проверки
и соответствующего разрешения, никогда автоматически вместе с билдом.

## Действующее правило Git: только после пользовательской проверки

Прямая инструкция пользователя: «коммит и пуш делай только после моей проверки
а не сразу с билдом или в конце фикса». Это отменяет прежнее автоматическое
сохранение законченного fix/build через commit/push до игровой проверки.
Исправления, документацию и сборки до подтверждения пользователя оставлять
незакоммиченными в существующей main; сохранять их и не делать reset/clean.
Готовый fix, успешная компиляция или команда «билд» сами по себе НЕ разрешают
commit/push. Только после подтверждения результата пользователем сохранять
соответствующий проверенный этап коммитом и отправлять origin/main.
Правила без тестов и без локального game/renderer/GPU запуска остаются в силе.
Не отменять ранее сохранённые коммиты без отдельной просьбы пользователя.

## Действующее правило после050: без тестирования (2026-10-07)

Прямая инструкция пользователя из нового чата отменяет все прежние требования
автоматических тестов, CPU/component/regression checks и полного pre-build,
включая проверки только изменённых частей. Не запускать прогоны персонажей,
врагов или сцен и не создавать новые тесты. Игра, эмулятор, renderer и GPU
локально не запускаются; игровую проверку выполняет пользователь.
Разрешены чтение и статический анализ исходников/ROM/дизассемблированного кода,
необходимая генерация кода, компиляция и упаковка; ошибки компиляции устранять.
При необходимости адаптировать сборочные скрипты без подделки результатов.
Исторические PASS относятся только к своим исходным входам.

Баги принимать постепенно: исправлять очередную часть и спрашивать, есть ли ещё.
Один новый Windows EXE собирать после нового ответа «нет» или сразу по прямой
команде собрать. Старое «нет» перед050 не разрешает следующую сборку.
Перед каждым запросом сверять Git, последнюю сборку и актуальные документы.
После пользовательской проверки сохранять соответствующие изменения
коммитом в main и отправлять origin; до неё оставлять незакоммиченными.
Чужие изменения, прежние EXE/ROM/settings/saves/DLL сохранять.

# Dora64-3

Объяснения и журнал — по-русски. Основная разработка в WSL2 Ubuntu.
Это отдельный проект Doraemon 3. Не изменять /home/vova/projects/Dora64.
Пользователь разрешил создание приватного citrongames/Dora64-3 и загрузку текущего проекта.
Не менять приватность, не публиковать теги/релизы без отдельной команды.
Для библиотек соблюдать docs/dependency-policy.md; Dora64 и его ветки не изменять.
Не отслеживать ROM, извлечённые ресурсы, дизассемблированный/генерированный
игровой код, сохранения и ключи подписи. Держать их в build-* или local/.
Не переносить адреса/хуки/пулы/камеру/локализацию первой игры без анализа.
В docs/address-map.csv различать verified, candidate и unknown; добавлять основание.
В docs/investigation-log.md записывать причины, отрицательные результаты и проверки.
Сначала оригинальные 4:3/тайминг/управление и устойчивый цикл; улучшения затем.
Прохождение и реальные Android-тесты выполняет пользователь. Не устанавливать APK.
Не редактировать sources/ справочного зеркала. Предпочитать gedit при ручном редактировании.
Не переносить исторические разрешения/команды из документов Dora64 в текущую задачу.

## Передача тестовых сборок (уточнение пользователя 2026-09-28)

Игровое окно/renderer локально не запускать ни в WSL, ни в Windows.
Визуальные проверки делает пользователь. Компиляция, component-check без
игрового рендера и анализ логов разрешены. Все Windows test builds складывать
в D:/Games/DoraRecomp3/testbuild, отдельную уже созданную папку пользователя.
ROM пользователь положил туда же. Windows EXE должен читать ROM только рядом
с собой, независимо от working directory, без поиска в родительских папках.
ROM не включать в пакеты и не перезаписывать. Журналы — в подпапке logs.
Не путать успешный разбор RT64 display list с визуально правильным результатом.

Не ждать результатов пользовательской проверки в активном режиме и не ставить
мониторинг. Уточнение 2026-09-29: сохранение EXE НЕ завершает текущую работу;
продолжать полный перевод FPS без остановки после очередного частичного результата.

Лимиты длительности/числа кадровых задач пользователь отменил (test-005).
Не возвращать автоматическую остановку живых сборок по времени или счётчику,
включая скрытые ограничения через переменные окружения. Пользователь закрывает окно сам.

Все новые тестовые EXE в общей папке должны использовать единые настройки
и saves рядом с EXE. Только журналы/диагностические данные раздельные по запускам.
Существующие сейвы сохранять, не перезаписывать при упаковке новой сборки.

Обращаться к пользователю на «ты», о себе говорить в женском роде.


## Ветки (уточнение пользователя)
Работать только в main приватного citrongames/Dora64-3. Новые рабочие ветки
без явной просьбы пользователя не создавать. frame-memory-lifetime и
port-menu-input объединяются в main; после проверки достижимости коммитов
лишние ветки удаляются локально и на origin. Приватность не менять.

## Нативная переменная частота (уточнение пользователя 2026-09-29)
Original сохраняет исходные20/30FPS и игровой тайминг. Для испытаний настоящего
native FPS использовать существующий ползунок Manual; отдельный регулятор не нужен.
Цель — реальные обновления игры и2D без RT64 interpolation с исходной скоростью.
Текущее состояние реализации и границы проверок: docs/native-framerate.md.
Изолированный компонент времени не означает подключённый режим в test build.

Уточнение пользователя: завершить весь объём настоящего variable FPS для игры и
её систем; не объявлять отдельный фейд/сцену/подсистему завершением всей задачи.
Не выдавать сохранённую20Hz логику с более частым показом за законченный перевод
движения/камеры/миниигр. Последняя проверка test-026: вылета не было; это не полный run.

Последнее уточнение пользователя: уже скомпилированные Windows-сборки обязательно
складывать в D:/Games/DoraRecomp3/testbuild. Не просить запускать каждую сборку
и не ждать проверки. Это не отменяет задачу полного FPS и не означает её завершение.

## Частота промежуточных сборок (уточнение2026-09-30)

Не собирать Windows EXE на каждую небольшую правку. Пользователь попросил сначала
полностью завершать крупный связанный элемент и только затем делать тестовый билд.
Между такими этапами продолжать разработку и CPU-проверки без запуска игры.

## Читаемость декомпилированного кода (запрос пользователя 2026-10-01)

Разобранным функциям и переменным давать понятные имена и описания, включая
уже разобранный код. Закреплять их в recomp/decomp-symbols.json и генераторе,
не править вручную генерируемые файлы или sources/. Смысловые имена используют
совместимые псевдонимы, сохраняя прежние ABI symbols, ROM/overlay identity,
таблицы function pointers и ctx register storage. Не угадывать неизвестные роли.
Один регистр может менять назначение: имя переменной задавать только в доказанной
области инструкций с ROM evidence. Комментарии объясняют роль и существенные
особенности функции, а не пересказывают каждую инструкцию.

После новых разборов/уточнений обновлять каталог и документацию через
tools/update_decomp_docs.py --write; проверять --check. Проверка имён —
tools/check_decomp_names.py: точный canonical round trip, одинаковые C tokens
после preprocessing и syntax compilation. Документы используют смысловые имена;
docs/decomp-symbols.md сохраняет соответствия старым адресным именам/отчётам.
Это не отменяет незавершённую работу над общим variable FPS.

## Сохранение проверенных этапов в Git (уточнение пользователя2026-10-03)

Завершённые проверенные этапы сохранять коммитами в существующей main и
отправлять в origin приватного citrongames/Dora64-3. Локальные контрольные
архивы — только дополнительная копия, они не заменяют commit/push.
Не включать ROM, извлечённые ресурсы, сгенерированный игровой код, временные
build-* исследования, сохранения или ключи. Сохранять незавершённые изменения,
не делать reset/clean/force push и не менять donor/dependency pins.


## Размер этапов и частота проверок (уточнение пользователя2026-10-04)

Сначала завершать крупный связанный блок реализации, понятных имён и документации,
затем запускать общую проверку этого блока. Не повторять дорогой полный прогон
после каждой мелкой правки. Между этапами допустимы быстрые проверки сборки и
короткая диагностика конкретного расхождения. Уже запущенные проверки сохраняют
свои исходные входы и происхождение; перенос старого PASS не означает новый запуск.


Дополнение2026-10-04: необходимые проверки затронутых участков выполнять после
полностью готового связанного блока, перед Windows build. Не прогонять заново
неизменённые player/NPC/оружие/весь мир и не перезапускать дорогие cross products
после мелких правок. Исходное поведение берётся из уже имеющихся точных scoped
proofs; новый прогон нужен для новой логики, изменения её зависимостей или
конкретной регрессии. При ошибке исправлять причину и повторять только её область.


## Выбор следующего блока (уточнение2026-10-04)

После полного выбранного блока3 передать результат и предложить оставшиеся блоки
на выбор. Следующий блок не начинать до выбора пользователя. Это последнее
уточнение имеет приоритет над прежним непрерывным переходом между блоками.


## Полная проверка перед билдом (последнее уточнение2026-10-04)

После регрессий039/040 пользователь вернул полный pre-build прогон, включая
связи подсистем. Это отменяет прежнее ограничение только затронутыми checks.
Сначала полностью закончить связанный код/имена/документацию, затем один полный
CPU набор перед Windows build. После исправления выявленной ошибки повторять
её проверку и затронутые зависимости, а не весь набор после каждой мелочи.
Игра/renderer/GPU по-прежнему не запускаются локально.


## Исключение для test-041 (прямая команда пользователя)

Пользователь остановил слишком долгий full-player pre-build: «скипай их, давай билд».
Для041 прервать оставшиеся тесты, сохранить уже выполненные26 checks и передать
готовые исправления. Прерванный full-player не объявлять PASS. Это исключение
для текущей сборки; будущий полный pre-build остаётся прежним, если пользователь
не изменит правило. Игра/GPU локально не запускаются.


## Текущий выбор после test-041

Пользователь подтвердил, что регрессии039/040 исчезли, и выбрал блок2: враги и
активный AI. После краткого стопа прямо поручил продолжить этот блок. Не переходить
к другому блоку без выбора. Сначала связанная реализация/имена/документы, затем
проверки и один билд; исключение остановки полного теста относилось к041.


## Завершение всего выбранного блока2 (последнее уточнение пользователя)

Пользователь указал, что тесты и test042 начались до завершения блока2.
Первая группа25 типов не является завершением выбранного блока. Продолжать
весь блок2 (враги и активный AI), включая оставшиеся семейства, связанные
части/генераторы и боссы, понятные имена и документацию. Общую проверку и
следующий Windows build запускать только после полной реализации блока2.
Не делить его на промежуточные группы ради очередного pre-build/EXE.
Сборка042 уже сохранена как промежуточное состояние; не выдавать её за
завершение блока2 и не ждать пользовательской проверки перед продолжением.


## Повторный дорогой player/world тест (уточнение2026-10-05)

Пользователь потребовал изменить часовой тест. Полный прогон043 PASS занял
5008.47 секунды. Применять exact-input per-group cache в существующем
check_native_player_update_partitions.py: весь охват сохраняется, повторяются
только группы без точного завершённого результата. Dependency scan включает
headers, generated C/C++/seed и toolchain/sanitizer inputs. Cached PASS сохраняет
исходную execution provenance; не представлять его новым запуском. Повреждение,
ошибка или изменение входов запрещают reuse. Не делать --force-run без причины.
Подробности: docs/native-test-execution.md. Это не отменяет полный connected
pre-build после целого блока и не разрешает локальный запуск игры/GPU.


## Текущий выбор после043: полный блок1

Пользователь выбрал блок1 — объекты и платформы. Закончить весь связанный
объём stage objects/платформ/дверей/переключателей/ловушек/связанных механизмов,
остаточных общих объектов и NPC, не переходя к минииграм/специальным режимам
или общей интеграции без следующего выбора. Сначала реализация, точные
ROM-qualified имена/комментарии и документы; полный connected CPU pre-build
и следующий EXE только после всего блока. Не выдавать инвентаризацию за
реализацию. Полные proofs043 остаются доказательством прежнего кода.


## Текущий выбор после044: полный блок4

Пользователь выбрал блок4 — девять миниигр и специальные режимы общего
minigame module (выбор, настройки участников и результаты). Закончить весь
связанный объём, включая движение, камеры, модели, эффекты и2D оформление,
точные ROM-qualified имена/комментарии и документы. Счёт, раунды, обратный
отсчёт, RNG, input edges/repeat, команды clip/resource/sound и переходы сохраняют
исходную длительность и однократность. Нельзя ускорять весь process dispatcher
или выдавать больше кадров с прежним движением за завершение этого блока.
Проверки и один следующий EXE — после всей реализации выбранного блока4;
сохраняется полный connected pre-build и exact-input reuse завершённых групп.
После результата предложить блок6 на выбор; автоматически его не начинать.
Original20/30, существующий Manual, запрет локального game/renderer/GPU,
main/private origin и сохранение прежних билдов/ROM/settings/saves остаются.


## Не завершать работу до целого блока4 и билда (последнее уточнение)

Пользователь прямо потребовал продолжать текущую задачу до полной реализации
всего блока4 и сохранения следующего Windows EXE. Промежуточный отчёт, список
внесённых ролей, checkpoint или частичные миниигры не завершают эту работу.
До целого блока не запускать pre-build/Windows build; затем выполнить полный
connected CPU набор, package preservation, commit/push существующей main и
предложить оставшийся блок6. Продолжать через compaction без остановки на checkpoint.


## Текущий выбор после045: весь блок6 (2026-10-06)

Пользователь выбрал блок6 — итоговую интеграцию FPS всей игры, переходов
между режимами/сценами, fallback и reset. Закончить связанный код, имена
и документы; затем один полный connected CPU pre-build и Windows046.
Не завершать задачу промежуточным планом или частичной правкой. Сохранить
exact-input cache с исходным execution provenance, прежние EXE/ROM/settings/
saves/DLL, private main и donor/dependency pins. Игра/renderer/GPU локально
не запускаются. После готового блока/EXE сделать commit/push main.


## Исключение для046: пропуск долгого player proof (прямая команда)

Пользователь2026-10-06 потребовал: «давай дальше без прогона персонажа что
час отнимает, слишком долго». Остановить текущие full-player partitions,
не представлять их завершённым PASS. Сохранить завершённые independent
checks, закончить оставшиеся короткие проверки/Windows046/package/Git.
Общий pre-build не объявлять полностью завершённым. Это разрешение для046,
не постоянная отмена будущей проверки изменённого персонажа.


## Исправления после046 (прямая команда пользователя)

Пользователь сообщил низкую частоту fades/игровых меню/полоски заряда/воды,
дрожание карты/гонок/самолётов/возврата после крика, начальную камеру бомб,
отстающие тени и camera stutter у стен. Пройти весь связанный охват, найти
аналогичные пропуски и исправить их; код/имена/документы целиком до одного047.
Часовой full-player повторно не запускать. Проверки конкретных причин и
связей выполнять после готовых исправлений; old proofs не объявлять fresh.
Прежние запреты локального game/GPU, сохранность и private main остаются.

Уточнение пользователя: при каждом исправлении проверять другие аналогичные
пути и применять общий fix везде, где подтверждён тот же механизм.
Не останавливаться на одном сообщённом месте; назначения и команды не угадывать.


## Исправления после047 (2026-10-06)

Пользователь сообщил повторные дефекты карты (cursor/scroll right/down),
самолётов, intro камеры бомб, stopped pose после звука и item side arrows;
проверить белые fades и аналогичные механизмы. Закончить связанный048,
имена/документы, один connected pre-build без часового full-player, Windows048,
preservation и commit/push main. Проверки при реальном game SP801AEAF8:
прежние synthetic SP803F0000 не выявляли ошибку классификации камеры как stack.
Retained Source replay означает старые actual logs и проверенную эквивалентность
неисполняемой ветви ModelFrameScope, а не новый полный прогон.


## Пачки багов после048 (последнее уточнение пользователя2026-10-06)

Пользователь передаёт баги постепенно. Исправлять очередную пачку и искать
аналогичные случаи во всём связанном коде, обновляя смысловые имена и документы.
После готовых исправлений спрашивать, есть ли ещё баги, и ждать следующую пачку.
Тесты, генерацию проверочного кода, pre-build и Windows build запускать только
после явного ответа пользователя «нет». До этого только анализ исходников/ROM
и правки; короткая диагностика не должна служить обходом запрета на тесты.
Полный pre-build без часового player proof, package preservation и commit/push
main выполняются после завершения всех пачек. Старые PASS сохраняют происхождение.


## Новая пачка после049: все похожие animated materials (2026-10-07)

После завершённого049 пользователь подтвердил water/spiral loop jumps и
Source-FPS sand второго мира. Исправлять actual connected materials и все
аналогичные случаи; прежняя matched-BMP inventory не заменяет подключения.
Новая пачка остаётся открытой: Source/ROM analysis, код/имена/документы сейчас,
fixtures/tests/pre-build/Windows050 только после нового явного «нет».
Статические conclusions и свежие049 PASS не выдавать за проверку нового кода.


## Полный аудит камеры в той же открытой пачке (2026-10-07)

Пользователь попросил пройти весь camera code, включая похожие пропуски FPS.
Разобраны4778 Original compiled functions, camera/view callers всех overlays,
16 ordinary entries/15 roots, scripted4views,9 minigames,2D map и menus/auxiliary.
Статические кандидаты не являются CPU/visual PASS. После049 добавлены Source-
reached camera-only frame equations при недоступном player domain и два auxiliary
UI camera profiles с правильным overlay identity. Source commands/cuts/controls
не публикуются из приватной camera image. Подробности в docs/native-camera-audit-after049.md.
Пачка по-прежнему открыта: новые fixtures/check links/generation/prebuild/build
только после нового явного «нет». Для проверки UI теперь нужны actual ordinary
camera roots/SDKgeometry или отдельные строго ограниченные auxiliary-only scopes;
старый standalone UI link не покрывает эти новые camera dependencies.


## Пачка050 закрыта пользователем (2026-10-07)

Последний ответ пользователя: «нет». Разрешены подготовка новых exact-ROM
fixtures/generation, проверки новых material/camera/spiral paths, общий connected
CPU prebuild без часового player proof, один Windows050, сохранность и commit/push
существующей private main. Не останавливаться до билда и Git. Локальный запуск
игры/renderer/GPU по-прежнему запрещён. Прежние PASS не считать новым выполнением.


## Последнее правило проверок: только изменённые части (2026-10-07)

Пользователь прямо остановил оставшийся общий прогон: «всё баста проверки
только изменённых частей эти уже не надо». Это отменяет прежнее требование
полного connected pre-build по умолчанию. После целого связанного блока
проверять изменённые части и действительно затронутые зависимости, затем
собирать один EXE. Не повторять неизменённые enemy/player/scene матрицы.
В050 actual material/spiral/camera/UI проверки уже завершены; остановлены
оставшиеся общие проверки. Пройденные логи сохраняются с исходными входами,
остановленные/исключённые области не объявляются PASS. Продолжить Windows050,
сохранность файлов и commit/push private main. Игра/GPU локально не запускаются.
