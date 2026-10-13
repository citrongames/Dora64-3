# Dependency policy

[dependencies.lock.json](../dependencies.lock.json) records the exact repositories
and commits for the runtime, renderer, SDL2 and their nested components.
`tools/bootstrap_dependencies.py` creates independent checkouts under `build-deps/`.
It checks revisions and preserves existing work rather than resetting it.

The generation tool is prepared by `tools/bootstrap_n64recomp.py` using
`tools/n64recomp/lock.json` and its verified patch. This tool has a distinct
role and pin from the runtime's N64Recomp dependency.

Game-specific changes live in this repository's generators and adapters.
Adapters produce isolated build-directory copies and keep dependency checkouts
unchanged. A separate Dora64 checkout is not needed to build Dora64-3.

Keep upstream licenses and attribution with redistributed components. Release
packages collect the license files, and the Source archive includes pinned
dependency sources under `dependency-sources/`.
