# Dora64-3

A native recompilation of **Doraemon 3: Nobita no Machi SOS!** for Windows,
Linux and Android, powered by N64Recomp, N64 Modern Runtime and RT64.

## Download

Get **version 1.0.0** from [Releases](https://github.com/citrongames/Dora64-3/releases).

| Platform | Package | Requirements |
| --- | --- | --- |
| Windows | `Dora64-3-1.0.0-Windows-x64.zip` | Windows 10/11, x64, current graphics driver |
| Linux | `Dora64-3-1.0.0-Linux-x86_64.tar.gz` | x86_64, glibc 2.43+, Vulkan, SDL2 and GTK3 |
| Android | `Dora64-3-1.0.0-Android-arm64.apk` | Android 7.0+, ARM64, Vulkan 1.0 |

**The game ROM is not included.** You need your own Japanese **N3DJ,
revision 0** ROM: a 16 MiB, big-endian `.z64` file. Its name does not matter.

```text
SHA-256: 410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602
```

## Getting started

**Windows:** extract the entire ZIP into a writable folder, place your `.z64`
ROM beside `Dora64-3.exe`, then start the program. Keep the bundled DLLs and
`assets` folder with the executable.

**Linux:** extract the archive, place your `.z64` ROM beside `Dora64-3`, then
run `./Dora64-3` from that folder. See the [Linux requirements](docs/building.md#linux)
for the required system packages. `python3 install-desktop.py` adds a launcher
to your applications menu.

**Android:** install the APK and select your ROM using the system file picker
on first launch. The application imports it into its own storage.

Press **Esc** on desktop, or tap the **gear** on Android, to open settings.
Use the Controls tab to configure keyboard, mouse and gamepads. The Android
Touch tab lets you move and resize the on-screen controls.

## Features

- Original frame rate and adjustable Manual FPS; the Modern profile uses 60 FPS.
- A modern camera with mouse, right-stick and touch controls in supported scenes.
- Widescreen support for gameplay, interface, backgrounds and split-screen modes.
- Japanese, English and Russian text and translated artwork.
- Autosave when returning to the stage-selection map.
- Up to four gamepads and separate keyboard bindings for each player.
- Graphics, audio, controls, cheats and Modern/Original profiles in the settings menu.
- In-app reset without restarting the program.

[User guide](docs/user-guide.md) · [Build from source](docs/building.md) ·
[Release notes](docs/release-notes.md) · [Documentation](docs/README.md)

## Building

Clone this repository, install the prerequisites for your platform, then run:

```sh
python3 tools/build_release.py --rom /path/to/original.z64
```

On Windows, use `python` from an x64 Visual Studio developer shell.
For Android, add `--platform android` and provide your signing configuration.
The [build guide](docs/building.md) covers prerequisites, Android signing,
incremental builds and packaging.

Dependencies are pinned in [dependencies.lock.json](dependencies.lock.json).
They are downloaded into `build-deps`; a separate Dora64 installation is not required.
Recompiled game and audio sources are generated locally from your ROM.

## License and credits

Dora64-3's own code is licensed under **GPL-3.0-or-later**; see [LICENSE](LICENSE).
Third-party components retain their original licenses and notices. The game,
characters and original game data remain the property of their respective owners.

Developed by **citrongames**, with assistance from **ChatGPT and OpenAI Codex**
in programming, analysis, localization and documentation.

Thanks to the contributors to **N64Recomp**, **N64 Modern Runtime**, **RT64**,
**Plume**, **SDL2** and the original **Dora64** project.
See [NOTICE.md](NOTICE.md) for attribution and third-party licensing.
