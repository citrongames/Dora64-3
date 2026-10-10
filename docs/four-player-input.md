## User acceptance — 2026-10-10

The owner confirmed four gamepads work in test110 and authorized commit/push.
Shared-keyboard play and exhaustive reconnect/FPS coverage were not separately
reported. No automated tests or game execution were performed by the agent.

# Four-player PC input

## Behaviour

Controls has Player1–4 tabs. Each player has independent keyboard bindings,
gamepad bindings and stick dead zone. Player1 imports the existing assignments;
Players2–4 start with standard gamepad bindings and all keyboard keys unbound.
Keyboard assignments can share one physical keyboard, or be combined with pads.
A secondary N64 port is present when its gamepad is attached or any usable game
key is assigned. Clearing all such keys and unplugging the pad removes that port.
Player1 remains present for the keyboard, matching earlier builds.

SDL opens up to four controllers, assigned in discovery order. Existing slots
are never compacted when a pad disconnects. Reconnection prefers the prior slot
using GUID plus serial/path where SDL supplies it; otherwise a free slot is used.
Controller order across full application restarts follows SDL discovery order;
independent control settings persist by player number. Player LEDs use that number
where supported. Devices beyond the first four free slots are ignored.

Esc opens the common PC menu. Back/Select from any connected pad can open/close it;
the opening pad owns gamepad menu navigation, with a connected-pad fallback.
Only the selected player's pad can supply a captured binding, and disconnecting
or replacing that pad cancels capture. Reset defaults affects only the selected
player. Key conflicts swap only inside that player's map, so shared-keyboard
players do not overwrite one another's assignments. Esc/Back remain reserved.

All players are silenced while the PC menu is open. Held controls are quarantined
per device/key on menu close, loss of focus and hotplug. Keyboard always requires
focus; background gamepad input remains a global choice. Modern camera/mouse
input stays with Player1; original minigame camera rules are unchanged. The two
modern-camera zoom-only actions are omitted from Player2–4 tables.

The bindings table uses the remaining tab height with space for its reset button
and the main footer, so the extra player-tab row does not restore the test108
whole-tab scrolling issue.

## Settings compatibility

`port_input_settings.json` version4 keeps Player1 in the existing top-level
`bindings` and `stick_dead_zone_percent` fields. Additional players are stored
under `players.2`, `.3`, `.4`, each with those same fields. Existing custom Player1
assignments are preserved; absent secondary-player settings use blank keys and
default pads. Older EXEs continue using their familiar Player1 fields and preserve
the extra object. Packaging does not rewrite settings or saves.

## Original and Native path evidence

N3DJ ROM SHA256:410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
The original functions were compared against the ROM/function fingerprints:

| Function | VRAM | Role |
|---|---|---|
| controllers_init | 800301B0 | Initializes four channels |
| controller_read | 80030324 | SI read, gated by Player1 presence |
| controller_accumulate | 80030380 | Four-port buttons/sticks |
| controller_latch | 800305E0 | Per-port held/edge arrays, disconnected clearing |
| controller_status | 8003C1B8 | Rebuilds all four presence bits from OS status |

The original presence byte is801A48B4. Status queries can stop with the game's
peripheral polling disabled, so the existing read-entry adapter now compares
all four host bits and calls the original status function on a topology change.
It does not fabricate status or write guest controller arrays directly.
The hardware PIF backend already accepts four ports; callbacks now supply four
separate snapshots instead of rejecting every port except0.

The race participant update801C7170 indexes the held array801A9504 by player;
other minigames have the same per-player access (e.g.801C4D8C/801C6334/801C66B4
in their distinct minigame overlays). The existing NativeInputScope already
handles all four ports,36 latch words and8 axes, consuming each player's edges
once. No new guest instruction patches or generated-code edits are required.

## Verification limits

Source/ROM inspection and incremental compilation only. No tests, automatic CPU
checks, game, emulator, renderer or GPU execution. The owner must verify actual
multi-device input, keyboard sharing, reconnects and multiplayer selection in
Original and Manual FPS. Original HUMAN/CPU selection remains game-owned;
connecting controllers does not force CPU characters into human control.
Commit/push waits for owner verification and explicit authorization.
