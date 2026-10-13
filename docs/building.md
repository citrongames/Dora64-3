# Building Dora64-3

Build commands are run from the repository root. You need your own Japanese
N3DJ revision 0 ROM in big-endian `.z64` format; see the [README](../README.md)
for its SHA-256. The build does not modify the ROM.

## Common prerequisites

- Git, Python 3.10 or newer, CMake 3.20 or newer, and Ninja.
- A C++20 compiler: Visual Studio 2022 on Windows or a recent Clang/GCC on Linux.
- Internet access for the initial dependency and tool downloads.

```sh
git clone https://github.com/citrongames/Dora64-3.git
cd Dora64-3
```

## Windows

Install Visual Studio 2022 with **Desktop development with C++**, the Windows
SDK and its CMake/Ninja tools. Install Python 3.10+ and Git, then open the
**x64 Native Tools Command Prompt** or an x64 Visual Studio developer PowerShell.

```powershell
python tools/build_release.py --platform windows --rom 'C:\Games\original.z64' --jobs 6
```

Output: `build-local/windows/Dora64-3.exe`. The build also copies SDL2 and
shader-compiler DLLs beside it. Copy the repository's `assets` folder beside
the executable when running an unpackaged build, or create a package:

```powershell
python tools/build_release.py --platform windows --skip-bootstrap --package-directory dist
```

The packager locates the Visual C++ runtime in your Visual Studio installation.
Use `--crt-directory` to provide a different runtime folder.

## Linux

Install a compiler and the SDL2, GTK3 and Vulkan development packages.
For Ubuntu:

```sh
sudo apt install build-essential clang cmake ninja-build git python3 pkg-config \
    libsdl2-dev libgtk-3-dev libvulkan-dev
python3 tools/build_release.py --platform linux --rom /path/to/original.z64 --jobs 6
```

Output: `build-local/linux-release/Dora64-3`. Package it with:

```sh
python3 tools/build_release.py --platform linux --skip-bootstrap --package-directory dist
```

The distributed Linux 1.0.0 binary requires **glibc 2.43 or newer**, libstdc++,
SDL2, GTK3 and a working Vulkan driver. On Ubuntu 26.04:

```sh
sudo apt install libsdl2-2.0-0 libgtk-3-0t64 libstdc++6 libvulkan1
```

It is a native tar archive, not an AppImage. Building from source on another
distribution uses that system's ABI and libraries. Keep `assets` beside the
executable. The packaged `install-desktop.py` registers the extracted copy
in the current user's applications menu.

## Android

Install JDK 17, Android SDK platform 36, build-tools 35.0.0,
NDK **27.0.12077973** and CMake **3.22.1** through Android Studio or `sdkmanager`.
The application targets ARM64 and requires Android 7.0/API 24 and Vulkan 1.0.

Set `JAVA_HOME` to your JDK 17 directory and `ANDROID_HOME` to the SDK directory.
The build creates the small host `file_to_c` utility using your host C++ compiler;
set `DORA3_HOST_FILE_TO_C` to use an existing copy.

Create an independent signing key once:

```sh
python3 tools/create_android_signing.py --keytool "$JAVA_HOME/bin/keytool"
```

Keep the generated `build-local/signing` directory private and backed up.
Use the same key for future updates. The key and its passwords are excluded
from source control and release archives.

```sh
python3 tools/build_release.py --platform android --rom /path/to/original.z64 \
    --signing-properties build-local/signing/release-signing.properties --jobs 4 \
    --package-directory dist
```

Output: `android/app/build/outputs/apk/release/app-release.apk` and a versioned
APK in `dist`. The release configuration disables debugging, native debug
information, diagnostic journals and profiling.

An existing signing properties file must contain `storeFile`, `storePassword`,
`keyAlias` and `keyPassword`. `storeFile` should be an absolute path. Set
`DORA3_SIGNING_PROPERTIES` instead of the command-line option if preferred.

## Incremental builds and generated sources

The build script downloads the exact dependency revisions, builds N64Recomp
and generates CPU/audio sources on the first run. It reuses valid generation
receipts on later runs. `--skip-bootstrap` skips dependency/tool preparation
when those are already available.

If generation inputs have changed, select a new directory with
`--cpu-directory build-local/startup-new` or
`--audio-directory build-local/audio-new`; keep earlier outputs if needed.
Do not edit generated C or RSP files. Changes to ordinary native source files
use the existing incremental compiler cache.

The equivalent manual sequence is:

```sh
python3 tools/bootstrap_dependencies.py
python3 tools/bootstrap_n64recomp.py --jobs 6
python3 tools/generate_startup.py --rom /path/to/original.z64 \
    --output-dir build-local/startup --no-diagnostics
python3 tools/generate_audio_rsp.py --rom /path/to/original.z64 \
    --output-dir build-local/audio-rsp
cmake -S . -B build-local/linux-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-local/linux-release --target dora3_startup -j6
```

The root CMake project builds the game by default. Research probes remain
available separately with `-DDORA3_BUILD_PROBES=ON`.

## Release packages

`--package-directory dist` creates a complete versioned package with runtime
assets, third-party licenses, instructions and checksums. It does not include
the ROM, personal settings, saves, signing keys or debug files.

You can also package an existing binary with `tools/package_release.py`:

```sh
python3 tools/package_release.py --platform Linux-x86_64 \
    --binary build-local/linux-release/Dora64-3 --destination dist
```

Windows packaging additionally needs `--crt-directory`. Android packaging
requires an already signed release APK. The version comes from the root
`VERSION` file. Existing release packages are preserved.

The Source archive also includes dependency snapshots under `dependency-sources/`.
The standard builder fetches the pinned build checkouts into `build-deps/`;
these snapshots are supplied for source availability and reference.
