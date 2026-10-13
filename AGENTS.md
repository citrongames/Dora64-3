# Maintainer instructions

## Scope

- Preserve existing work and use the current main branch. Do not rewrite Git history.
- Commit and push only after review and explicit maintainer authorization.
- Do not change repository visibility or publish releases without explicit authorization.
- Treat synced `sources/` references and the separate Dora64 project as read-only.
- Keep ROMs, extracted original game data, generated sources, binaries, saves,
  settings, diagnostic captures and signing credentials out of Git.

## Implementation

- Study the original code and locked ROM before changing game behavior.
- Preserve timing, events, collisions and resource ownership in Original and Manual FPS.
- Edit native code, generators or adapters; never edit generated C/RSP manually.
- Dependency versions are fixed by dependencies.lock.json. Adapt copies in the
  build directory; do not modify dependency checkouts or the separate Dora64 project.
- Game, emulator, renderer/GPU, automated runs and CPU checks must not be launched
  by the agent. Static inspection, necessary generation, compilation and packaging
  are allowed. Runtime verification is performed by the maintainer.
- After an implementation change, perform one incremental Windows Release build.
  Documentation-only and Git changes do not require recompilation.
- Preserve older binaries and all user data. Release defaults disable diagnostic
  journals, crash logging and graphics profiling.

## Communication and documentation

- Communicate with the maintainer in Russian, informally; use feminine self-reference.
- Public README, build instructions, package instructions and release descriptions
  are written in English. Describe the product directly, without conversation history.
- Preserve third-party licensing and attribution. The own-code license is GPL-3.0-or-later.
- The version is stored in VERSION; the current release preparation targets1.0.0.
