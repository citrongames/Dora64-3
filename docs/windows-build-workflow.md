# Windows build workflow

Install Visual Studio2022 with the C++ x64 tools, Windows SDK and CMake/Ninja,
plus Python3.10+ and Git. Open an x64 developer shell from Visual Studio.

```powershell
python tools/build_release.py --platform windows --rom 'C:\Games\original.z64' --jobs 6
```

The builder prepares pinned dependencies and generated sources, then uses the
incremental Release cache in `build-local/windows`. Output is `Dora64-3.exe`,
with the runtime DLLs beside it. Diagnostic journals and profiling are disabled.

Package the build with `--package-directory dist`; Visual Studio's runtime DLLs
are located automatically, or supplied with `--crt-directory`. The package
contains assets, licenses, instructions and checksums. It excludes ROMs, saves,
settings, signing credentials and debug files.

For complete commands and generation-directory options, see [building.md](building.md).
