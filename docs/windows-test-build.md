# test-109: correct desktop menu height

test108 accidentally copied Dora64's Android-only child-window reservation,
removing72px from the usable tab height and forcing Game/Controls to scroll.
The desktop branch calls drawContent directly. All five Dora64-3 tabs now
use that same desktop layout. Font size, menu dimensions, footer anchors and
the Controls binding table's own252px scrolling region are preserved.

The profile-description cleanup and eight-setting profile audit remain included.
See docs/port-menu-controls.md. Incremental compile/link succeeded; generated
code unchanged. No tests, CPU checks, game, emulator, renderer or GPU execution.
User visual verification is pending. No commit/push before user authorization.
Previous EXEs, ROM, settings, saves, DLLs and localization assets are retained.

## User acceptance — 2026-10-10

The owner confirmed the corrected menu displays properly and authorized
commit/push of the menu changes through109. The build receipt remains unchanged;
no new compilation or runtime execution was performed for acceptance.
