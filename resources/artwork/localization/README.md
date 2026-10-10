# Localized title artwork

The joined English/Russian Doraemon wordmarks are copied from the existing Dora64
translation pack with the owner's explicit request for continuity. Donor files
remain untouched. Six original256x15 strips form each256x90 source; Dora3 export
is168x58, nearest pixel grid. Dora3's original separate numeral3 stays unchanged.

The town subtitle EN/RU sources were produced with the built-in imagegen tool,
using the original184x56 plaque as an edit reference. Native exports preserve
184x56 and alpha, use nearest-grid conversion and a240-colour native palette.
The main title is never included in that edit.

Prompt EN: Replace Japanese lettering only with NOBITA'S TOWN SOS! Preserve grey
rooftops, silver beveled skewed plaque, red chunky italic letters with cream
highlights/dark shadow and low-resolution N64 pixel style. Whole plaque uncropped.
Prompt RU: Same invariants, exact lettering ГОРОД НОБИТЫ SOS!.
Follow-up on each: remove only blue outside background, preserve lettering/plaque
and output actual alpha transparency.

`tools/generate_localized_art.py` is the source for pixel-typed UI labels and
button geometry, using the shared hand-drawn donor atlas. It also exports these
title sources to their native game dimensions. Runtime partial-region composition
retains original ROM artwork around controller/inventory captions; no whole ROM
texture is distributed for those panels. Original ROM reference dumps stay local.
