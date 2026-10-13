# Attribution and licensing

Dora64-3 is developed by citrongames, with assistance from ChatGPT and OpenAI
Codex in programming, analysis, localization and documentation.

The project's own code is licensed under **GPL-3.0-or-later** (GNU General
Public License version 3 or, at your option, any later version). The full
license is provided in `LICENSE`. Existing notices and licenses attached to
third-party code and assets remain applicable.

## Components

- **N64 Modern Runtime** — GPLv3, including ultramodern and librecomp.
- **N64Recomp** — MIT; its tool notice is preserved in
  `tools/n64recomp/LICENSE.N64Recomp`.
- **RT64** and **Plume** — MIT, together with their separately licensed components.
- **SDL2** — zlib license.
- Additional dependencies include ELFIO, fmt, Rabbitizer, SLJIT, toml++, miniz,
  o1heap, xxHash, Dear ImGui, ImPlot, im3d, Vulkan headers and allocators,
  SPIRV tools, stb and zstd. Their original license texts accompany binary packages.
- The DirectX Shader Compiler carries its LLVM/Microsoft and third-party notices,
  included in `resources/licenses/DirectXShaderCompiler-LICENSE.txt` and release packages.

Exact repositories and revisions are recorded in `dependencies.lock.json`
and `tools/n64recomp/lock.json`. Modifications specific to Dora64-3 are kept
in this repository's adapters and generators.

## Dora64 provenance

Build infrastructure, Android integration, touch controls, fonts and localized
wordmarks were adapted from citrongames/Dora64 at revision
`680d81feb4c57309ba80536300d73725ea9dffaa`. Game-specific code and address
adapters are maintained separately for Doraemon 3.

Artwork provenance is documented in `resources/artwork/README.md` and
`resources/artwork/localization/README.md`.

## Game data

Doraemon, its characters, names and original game data belong to their respective
rights holders. This is an independent fan project. The project license does
not grant rights to the original game ROM or original extracted game assets.
The ROM is supplied by the user and is not included in the repository or packages.

## Source availability

Release source is provided as `Dora64-3-1.0.0-Source.tar.gz` alongside binaries.
It includes the project's build scripts, local adapters and pinned dependency
sources. The repository and build instructions are available at
https://github.com/citrongames/Dora64-3.
