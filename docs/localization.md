## test121: «Да» в подтверждении предметов и оружия

Добавлена пропущенная короткая строка ROM0CE414 / VRAM80216894 (scene1):
RU «Да», EN «Yes», оригинальные2 ячейки. Её напрямую рисуют оба меню через
801B6750: загрузки указателя801F2DFC и801F5644 подтверждены по ROM.
Общий механизм точного совпадения overlay/адреса/байтов уже подходит;
изменений кода, координат, выбора и новой генерации не требуется.
Соседние подтверждение и «Нет» уже присутствуют. Поиск отдельного японского
«はい» с терминатором по всему ROM нашёл именно эту единственную строку.
Статически проверены символы шрифта и длина. Игра и тесты не запускались.
Исправления NPC через120 сохранены, их принятие этим сообщением не заявляется.

# English/Russian localization (after accepted test105)

Game > Language follows Dora64: after profiles, before Autosave. The manifest
provides Original Japanese, English and Russian; `language.code` persists in
port_pc_settings.json. A missing preference uses the first supported SDL system
locale. Profiles do not change language. Cheats now use the English item glossary.

## Coverage

2,164 indexed text records: 2,152 translated into each language, plus 12 deliberately
untranslated alien-symbol utterances before Translation Konjac. Covers NPC/map
dialogues, 26 text-bearing scripts among the 30 cutscene resources350..379,
inventory/item/weapon descriptions, save/file prompts, character menus, minigame
rules/settings/results and credits. Item/weapon descriptions have230 contextual
records because reused Japanese fragments require different English/Russian lines.
Shared minigame lines are translated in their pointer-table order.

66 exact-ROM texture identities:64 English PNGs and66 Russian PNGs (the original
ITEM/WEAPON frame captions already are English). Includes title, files, pause,
inventory commands, participant/food labels, racing labels and result messages.
The main localized Doraemon wordmarks and dialogue font come from Dora64, read-only.
Title subtitle art has new EN/RU lettering; large banners, narrow names and buttons
have separate colour/outline/type profiles. Native dimensions/pixel grids stay fixed.
Two world recital signs retain original indexed material format and UV coordinates.

The corporate EPOCH/copyright logos, decorative blue wallpaper, numerical/button
symbols and the international GAME OVER graphic remain original. Voice-recognition
vocabulary is internal command data, not UI text, and is not translated. The legacy
Controller Pak service font is distinct from the EEPROM menus used by this port;
its dormant service screens are not claimed as localized.

## Runtime architecture and ROM evidence

Locked N3DJ SHA256:
`410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602`.
`research/localization-hooks.json` fingerprints complete original functions;
`tools/localization_patches.py` adds hooks during generation. Generated C remains
disposable. No donor addresses or guessed signatures are used.

The shared text paths are801B6750 (plain),801B6694 (glyph lookup),801B5FF0
(glyph drawing),801B713C/801B73E0 (NPC selection),801B6AFC/801B6B10 (script
binding/selection). Script loading801E3CE8 supplies verified ROM ranges. Context
sites801F3098/801F58C8 qualify item/weapon table slots. Choice site801B80F0 keeps
the original eight cells and cursor positions. Identity includes overlay ROM,
address and complete original bytes, because VRAM addresses repeat across overlays.

Latin/Cyrillic characters are paired into an original12px cell and drawn6px apart
from the donor12px atlas. Commands/arguments/colours and original script timing are
preserved. NPC lines have an explicit18-cell budget (19 only where original needs
it); subtitles retain their original cell budget so reveal time cannot expand.
Font and three independent text scratch slots reserve807F8000..80800000 outside
the spill heap but inside every Source/Native snapshot. Language is pinned for an
active NPC dialogue or bound script; switching affects the next dialogue/script.
Plain menus and sprites use the selected language immediately. Reset recreates
guest allocations after the runtime drains/discards the old heap.

Whole PNG sprites are substituted at8002A844 BEFORE original clipping/strip
division. They are converted once to nativeCI8 with an immutable extended-RDRAM
TLUT and guard row. The original1500-pixel strip budget fits beside the palette.
The SDK direct-colour branch emitsRGBA32 (FD18), so it is deliberately not used
with16-bit pixels. Original CI4/CI8 identity compares the full palette and texels;
actual palette lengths are honoured, including7,15 and237-colour resources.
Recursive drawing bypasses matching via immutable extended addresses; widescreen
and cadence hooks run once in the actual original draw. Partial panel regions
retain all other artwork from the user's ROM. The800282EC payload adapter covers
only the two explicit world signs, quantized against their original palettes.

This intentionally replaces the initially proposed RT64 hash packs. Sprite hashes
depend on clipped strips and can include a guard texel outside the bitmap. Matching
the complete ROM resource before stripping is stable without GPU dumping and needs
no RT64 changes. The donor-style `<language>/textures/*.png` layout is retained;
`texture_index.json` carries exact ROM identity instead of renderer hashes.

## test107: user-reported menu omissions and description fit

File-select function801BD9D0 copies original strings into two50-byte buffers,
801C6850 and801C6708;801BD970 draws these buffers through801B6750. The catalog
now records `copied_to` aliases for those exact strings. Lookup still requires
the active scene34 overlay and a complete original-byte match, including NUL.
Japanese buffers are untouched; translation happens only in the text scratch
slot during drawing, in both Original and Native modes. No new hooks/generation
or additional host simulation state are needed. The second-row choices retain
their original text anchors x88/x196 after the bell cursors x68/x180.

The save-card label is five separately drawn single-character strings at
ROM06FBF4..06FC04, with10px spacing, not a bitmap. Added GAMES / ИГРЫ in those
same cells. The shared minigame action atROM19606C (including both shooting
rules screens) now reads Shoot / Стрелять. Internal voice-recognition vocabulary
remains untouched.

All230 contextual item/weapon description lines (86 pages per language) now
have a12-cell /24-character limit instead of14cells. Reflow preserves the
original table line counts and wording is shortened where needed. Glyphs keep
their native size; the right border has room for glyph overhang and padding.
The static catalog validates the reduced budget for custom translations too.

## Validation and limits

Only static source/ROM analysis, catalog/asset lint, required generation and
incremental compilation/packaging are allowed. No tests, CPU checks, game, emulator,
renderer or GPU are run. Compile success does not establish in-game correctness.
The supplied catalogs preserve commands and glyph availability;520 recovered
pointer-table pages were checked against the100-token buffer. Full visual/layout
and translation-in-context review across gameplay remains the user's check.
On 2026-10-10 the owner confirmed test107 corrections work and authorized
commit/push of the localization through107. Full-game contextual review remains
an ongoing user activity, not an automated verification claim. Accepted105 and
earlier camera/FPS/widescreen/autosave fixes remain intact.
