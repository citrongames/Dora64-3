# Repository policy

The repository stores native source, generators, local adapters, ROM metadata,
localization, original project artwork, build scripts and documentation.

ROMs, extracted original game data, generated CPU/audio sources, compiled
binaries, local settings, saves, logs, memory captures and signing credentials
stay outside Git. Binary release packages are assembled separately.

`research/` contains inputs required by the game generators and build target.
Do not remove it wholesale when organizing documentation.

Dependencies use independent checkouts in `build-deps/` with exact revisions
from [dependencies.lock.json](../dependencies.lock.json). The N64Recomp host
tool has its own pin and patch in `tools/n64recomp/`.

Keep third-party notices and the project's GPL-3.0-or-later license. Supply
source and license files with binary releases. Use ordinary commits and preserve
history; publishing is a separate maintainer action.
