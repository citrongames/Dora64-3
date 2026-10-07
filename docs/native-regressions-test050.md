# Test050: итог исправлений материалов и камеры


Исправлены непрерывные циклы воды, waterfall и спирали выбора предметов.
Подключены промежуточные samples sand второго мира и связанных CI4/IA8
материалов. Camera audit4778 Original functions обнаружил зависимость обычной
камеры от player eligibility и два auxiliary Source-only пути; добавлены
независимые camera equations и правильные overlay/module identities.

Actual ROM35 sequences/339 pairs:674 Source transitions,658 scroll и1626
CI4/IA8 samples;35 paused/rebound owners. Real GIMO0A45/GIMM0A46:86 coupled
angle/scale samples. Camera/UI:192 blocked-player owners/16 ordinary modes,
220 auxiliary cases/4views, оба Source divider и60..360 FPS. Source RAM,
time/flags/context и ownership сохранены.

Пользователь закрыл пачку «нет», затем остановил оставшиеся общие проверки:
«проверки только изменённых частей эти уже не надо». Это последняя политика.
26 актуальных CPU scopes PASS, включая изменённые материалы/модель/камеру
и нужные связи;6 scopes исключены, полный suite не объявляется PASS.
Пройденный gravity log предшествует изменению inactive test-only fixture;
он сохранён, но исключён из свежего proof. Enemy был остановлен при компиляции,
scene transition/frame/story не повторялись; full-player исключён ранее.
Игра/renderer/GPU не запускались. Визуальная плавность и color/filter quality
ещё не подтверждены. Windows050 собран и сохранён; прежние файлы сохранены. Этап сохраняется в private main.


## Выполненные CPU scopes

- **integration**: PASS: 49 actual common-loop integration schedules, actual activation clock origin after held Original read, six Manual rates and both Source dividers, fractional modes, two completion boundaries, rate/reload/callback cancellation, unsupported fallback and quiescent mailbox reset; synthetic scene-clock/queue endpoints; no game/GPU. ([отчёт](../research/native-integration-check.json))
- **texture_animation**: PASS:35 actual ROM room-material cycles/339 frame pairs, 674 real Source transitions, 658 complete-cycle scroll samples, 1626 CI4/IA8 deformation samples, skipped/half-tile seams, native palettes/TMEM shape, all immutable assets, two frame banks and35 paused/rebound owners; no game/GPU. ([отчёт](../research/native-texture-animation-check.json))
- **minigames**: PASS: all9 minigame modules and3 common callback pairs; 1757 Source/Native cases, 10205 fractional slices;20/30Hz and Manual60/73/120/144/165/360; command-once, linked Euler motion, contact impulses, angle wrapping, model/draw input ownership and exact restoration; synthetic resources, no game/GPU ([отчёт](../research/native-minigame-check.json))
- **ui**: PASS: 192 reached Original ordinary camera owners with blocked player,16 mode entries, both Source rates/60..360 FPS, real collision geometry, independent camera policy, Source endpoint/whole-image/context restore, no gameplay double-step and held/resource/script guards. ([отчёт](../research/native-ui-check.json))
- **cutscene**: PASS: 154 connected cutscene cases, 506 exact Source endpoints, 1431 Native intervals, 18 ownership/policy guards; 52 actual motion functions, all four linked cameras, XYZ/rotation/scale/polar/jump/alpha, Original20/30 at60/73FPS, actual interpreter wait/cut/finish, Source restoration and full-RAM/context duplicate-time checks; ASan/UBSan; no game/GPU. ([отчёт](../research/native-cutscene-check.json))
- **model_animation**: PASS: 430 one-shot terminal pose/morph samples, ordinary loops, component/run/step guards, reverse control, Source flags/time/context and bank restoration. ([отчёт](../research/native-model-animation-check.json))
- **frame_time**: PASS: Manual 60..360 deadlines, Original/Display bypass, 20/30 Hz time conservation, mode changes, stalls/resume; 655360 original fade comparisons and fractional fades; 384 original animation comparisons, rate-independent native clip time, pause, clamp, multiple wraps and completion retention. All five original fade commands, inter-frame timestamp, no-op fractional retention, same-time commands and unknown-write rejection; rational Source command origins/reversal at12 FPS/divider settings with nonzero origin. No game/window/GPU. ([отчёт](../research/native-frame-time-check.json))
- **simulation**: PASS: 280 differential executions of seven original MIPS motion/camera helpers; Original and whole ticks unchanged; 12 20/30 Hz schedules at 60/73/120/144/165/360 FPS, 2036 real motion slices, exact discrete boundaries, constant-force kick/drift trajectory, approach/decay, long gaps, scene-reset cancellation and scope cleanup; no game/GPU. ([отчёт](../research/native-simulation-check.json))
- **camera_control**: PASS: 970 camera control/transition/coefficient cases; actual original angle helper, Original and full-step RAM equivalence; 60/73/120/144/165/360FPS, 20/30Hz; minimum distance20 unchanged; pitch dead-zone tolerance0.11 degrees; no game/GPU. ([отчёт](../research/native-camera-control-check.json))
- **camera_modes**: 801D5864 diagnostic_max_rate_spread=0 field=0 variant=0 ([отчёт](../research/native-camera-modes-check.json))
- **script_camera**: PASS: 3920 script-camera cases; four cameras, all four modes, six orbit channels, local channels and combined rotations; Original/whole-step RAM differential, durations 0/1/2/7/40, 20/30 Hz timing at 60/73/120/144/165/360 FPS, original trigonometry and eye geometry; no game/GPU. ([отчёт](../research/native-script-camera-check.json))
- **effects**: PASS: 200 rectangle/zoom/sprite cases; all four rectangle modes, loop/clamp sprite events, durations0/1/2/7/40; original whole-step equivalence and per-tick state/event comparison at60/73/144/360FPS and20/30Hz; real fractional zoom changes; no game/GPU. ([отчёт](../research/native-effects-check.json))
- **input**: PASS: 12 real VI accumulator/latch schedules at 60/73/120/144/165/360 FPS, four-port ownership, short presses, analog thresholds, Original handoff, disconnect/reconnect and scene reset; no game/GPU. ([отчёт](../research/native-input-check.json))
- **player_motion**: PASS: 168 drive partitions; 24 joint player/vector/gravity/contact/camera trajectories at 60/73/144/360FPS; Original equivalence; movement cache preserved across camera boundary; no game/GPU. ([отчёт](../research/native-player-motion-check.json))
- **process_time**: PASS: 2053 process dispatcher checks; all eight slots, states0..5, waits0/1/3, Original memory equivalence, repeated draw/simulation dispatch, delayed first activation; no game/GPU. ([отчёт](../research/native-process-time-check.json))
- **frame_bridge**: PASS: registered type2 drains during pause while unregistered type2 remains Original; real runtime queue full/requeue/interrupt gating; original game loop native/VI/task/pause dispatch; context and all odd FPRs preserved in FR=0/FR=1; stale scene/rate wakes; quiescent reset; update before bank reservation, scene/rate cancellation; 2000 concurrent deliveries ([отчёт](../research/native-frame-bridge-address-check.json))
- **frame_scheduler**: PASS: one queued wake; immutable token behind 32-bit message; two banks; publication and independent guest/host completion; stale scene/rate tokens; enqueue rejection; elapsed-time conservation under backpressure; Original bypass; concurrent completion order; original ROM VI gate and completion-before-swap order; native 60/73/144/165/360 scheduling builds original 2D fade display lists with extra alpha phases and unchanged duration. No game/window/GPU. ([отчёт](../research/native-frame-scheduler-address-check.json))
- **graphics_pump**: PASS: immediate game+host completion and bank reuse after both SP/DP caller tails (SP-first and DP-first); registered native framebuffer bypass, present after swap, type2 consumption and delayed host acknowledgement; original submit -> native-only wake -> original scheduler including full event queue; no synthetic VI/client broadcast; audio priority/yield, SP/DP completion, framebuffer gate, pause, late Original wake and context/thread-local isolation preserved ([отчёт](../research/native-graphics-pump-check.json))
- **loop**: PASS: resource barrier drains two banks with early guest completion/late SP retirement/late HostRead, preserves foreign FIFO, registers and scratch; original message loop + update + dispatcher + full frame build/submit + image draw; shared native controller; Manual 60/73/144/360, Original, pause, original white fade commands and scene selection, drain before callback/resource replacement; 288 native white-fade builds with 255 distinct alpha values; 18 last-interval builds; 348 actual RDP fade-alpha comparisons; request state1; 76 discrete updates, 1103 native builds, 2 Original builds, 232 VI events; no GPU ([отчёт](../research/native-loop-check.json))
- **scene34_draw**: PASS: scene34 image branch original draw closure; CI4/CI8 headers, 3 sizes, 2 banks, alpha 0/128/255; 802 actual 2D command lists at 60/73/144/165/360 with native fade; all guest RAM outside DL/cursor/stack unchanged; no renderer or live scene activation ([отчёт](../research/native-scene34-draw-check.json))
- **session**: PASS: in-progress Original read delays native entry; late reads denied; exit waits both completions; cancellation/stale reservation/pause/reset; actual sleeping producer wakes on deadline, mailbox receive, bank release and rate change; stops/joins with pending wake and restarts idle ([отчёт](../research/native-session-address-check.json))
- **present**: PASS: all six completion orderings, early/duplicate/stale callbacks, two-bank backpressure, concurrent host/game completion; exact runtime VI oracle (16 combinations), copied framebuffer/mode, no VI state advance, enqueue rejection, real worker acknowledges after reads and drops late periodic actions ([отчёт](../research/native-present-address-check.json))
- **scene_objects**: PASS: scene 3504 frozen/current Original and 10512 Native schedules across all292 ROM variants / 289 kinds; fresh replay=292 kinds/14016 cases, retained unchanged=0 kinds; 840960 fresh pool endpoints, 7461768 intervals, 7461768 zero-dt checks, 9184 genuine contacts; 289 scene/NPC kinds including platforms, doors, switches, traps, event actors and effects, actual constructors/retirement on synthetic assets; no game/GPU. ([отчёт](../research/native-scene-object-check.json))
- **names**: PASS: 2690 descriptions; 2359 semantic function names; 5851 register scopes; 96 C units with identical preprocessed tokens and successful C17 syntax compilation; unchanged ABI/register storage; no game/GPU. ([отчёт](../research/decomp-names-check.json))
- **overlay**: PASS: 49 actual ROM overlay ranges, verified DMA/reload, identical VRAM replacement, partial overwrite, parent invalidation, scene unload and boot reset; ASan/UBSan/LSan; no game/GPU. ([отчёт](../research/overlay-load-state-check.json))
- **docs**: актуальные смысловые имена и документация совпадают.

## Ограничения

- Latest user instruction stopped remaining unchanged scopes; only changed parts and necessary dependencies are required.26 current valid CPU scopes PASS;6 scopes excluded, not a full-suite PASS.
- Enemy compilation interrupted; scene-transition/scene-frame/story scopes not rerun; full-player excluded earlier. Gravity runtime unchanged; its completed log predates an inactive test-only camera fixture addition and is excluded from fresh proof.
- Actual ROM bitmap cycles and GIMO/GIMM0A45/0A46 plus camera math/Source commands run in CPU fixtures; no local game/renderer/GPU or full playthrough.
- CI4 palette sampling retains original TMEM shape; CPU checks do not confirm real water/sand filtering or color quality.
- Blocked-control ordinary camera16 modes and auxiliary4views/Source restore are covered; real in-game camera smoothness remains user visual QA.

## Сохранённый Windows build

EXE: D:/Games/DoraRecomp3/testbuild/Dora64-3-test-050.exe.
SHA256: 1a945cd64b0a291f2fc25194f3dbeaa6cfd392b5c59f2a6b69d68ca4e3c6efb1.
MSVC build и package завершены; хеши всех прежних файлов совпали, включая ROM, настройки, saves и shared DLL. Игра не запускалась.
