## test115 accepted by the user (2026-10-11)

User confirmed the upper wall continuation and authorized commit/push to main.
This completes acceptance of the minigame widescreen corrections through115;
earlier number-game and fruit-field fixes were confirmed in112–114. The build
receipt remains as built; pending wording below records the delivery state.
No new build or agent game/test execution for this Git step.

# test-115: fruit/voice minigame upper wall continuation

User accepted114 ground/checker continuation. Added the upper wall from X800
to2800 at original Y0..400 and Z=-500. Its original repeating texture keeps
its scale and phase. Wall material list is a separate immutable copy.

Retained the accepted ground and checker field continuation:
Twenty new columns retain the original100-unit cell size and alternating colors;
two grass strips fill the outside edges. Original field, red goal line, actors
and gameplay bounds stay unchanged. Accepted test113 number-game ground/sky
and grenade arena corrections are retained.

Immutable material-list/vertex copies preserve extended-RDRAM pointer tags.
Original aspect uses the original scene; Original and Native/Manual share the
wide extension. See docs/widescreen.md and widescreen-fruit-extension.json.
Generation: startup-wide-fruit114-v1; incremental compile/link succeeded.
No tests, CPU checks, game, emulator, renderer or GPU execution. User visual
verification pending; no commit/push until explicit authorization.
Previous EXEs, ROM, settings, saves, shared DLLs and assets retained.
