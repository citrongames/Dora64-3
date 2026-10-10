## User acceptance — 2026-10-10

User tested four gamepads in test110, confirmed they work and authorized
commit/push to existing main. This supersedes the pending authorization below.
No additional build or agent-run gameplay/tests; the receipt remains as built.

# test-110: four-player gamepad and keyboard input

Up to four SDL gamepads supply independent N64 ports. Controls has Player1–4
tabs, each with keyboard/gamepad bindings, dead zone and its own default reset.
Player1 keeps previous assignments; Players2–4 start with blank keyboard maps
and standard gamepad maps. Assigning keyboard keys also connects a secondary
port, allowing several players on one physical keyboard.

Existing device slots remain stable when another pad disconnects; reconnects
prefer the previous device identity. Any pad can open the PC menu, while binding
capture reads only the selected player's device. Held inputs are quarantined
per device across menu/focus/hotplug changes. Modern camera remains Player1-only.
Original status queries refresh all four port bits after hotplug/key setup.
Existing four-port Original/Native latch and minigame paths are reused.

See docs/four-player-input.md for ROM evidence, settings migration and limits.
Incremental Windows compile/link succeeded; generated code unchanged. No tests,
CPU checks, game, emulator, renderer or GPU execution. Real multiplayer, reconnect
and Original/Manual FPS verification remains the owner's task. In the original
participant menu, choose human players as usual; CPU choices are not overridden.
No commit/push before user verification and explicit authorization.
Previous EXEs, ROM, settings, saves, shared DLLs and localization assets retained.
