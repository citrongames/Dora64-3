# Dora64-3 1.0.0

The first release of Dora64-3 brings **Doraemon 3: Nobita no Machi SOS!**
to Windows x64, Linux x86_64 and Android ARM64 through native recompilation.

## Highlights

- Original frame rate and adjustable Manual FPS, with 60 FPS in the Modern profile.
- Modern camera controls using mouse, right stick and touch in supported scenes.
- Widescreen gameplay, adapted interface, backgrounds and split-screen modes.
- Japanese, English and Russian localization, including translated artwork.
- Autosave at the stage-selection map, four-player input and configurable controls.
- In-app graphics/audio settings, Modern/Original profiles, reset and optional cheats.
- Android touch controls with a layout editor and physical gamepad support.

Animation transitions, fractional-frame motion, boss state changes, race braking,
sprite transparency and the ending's widescreen presentation are addressed
through the port's shared runtime and scene adapters.

## Downloads and requirements

| File | Requirements |
| --- | --- |
| `Dora64-3-1.0.0-Windows-x64.zip` | Windows 10/11 x64 and a current graphics driver |
| `Dora64-3-1.0.0-Linux-x86_64.tar.gz` | glibc 2.43+, SDL2, GTK3, libstdc++ and Vulkan |
| `Dora64-3-1.0.0-Android-arm64.apk` | Android 7.0+, ARM64 and Vulkan 1.0 |
| `Dora64-3-1.0.0-Android-arm64.zip` | APK with instructions and licensing files |
| `Dora64-3-1.0.0-Source.tar.gz` | Project and pinned dependency sources |

Each desktop archive includes the runtime assets and licensing files. Release
builds omit diagnostic journals, profiling and debug symbols. Checksums are
provided in `SHA256SUMS.txt`.

## Required game data

The ROM is not included. Supply your own Japanese **N3DJ revision 0** ROM,
16 MiB, big-endian `.z64`, with SHA-256:

```text
410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602
```

On Windows/Linux, place it beside the executable. On Android, select it on
first launch. Its filename does not matter.

[Getting started](../README.md#getting-started) · [User guide](user-guide.md) ·
[Build instructions](building.md) · [Licensing and attribution](../NOTICE.md)
