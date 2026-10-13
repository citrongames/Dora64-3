# Release packaging

The root `VERSION` file is the version source for Windows metadata, Android
versionName/versionCode and archive names. Release packages are built with
diagnostic journals, graphics profiling and debug symbols disabled.

Build each platform with `tools/build_release.py` and use
`--package-directory dist` to create its archive. All packages include license
texts, attribution, user instructions and SHA256 checksums. Android also
provides a standalone signed APK. Existing packages are preserved.

Create the source archive with `python3 tools/package_source.py --destination dist`.
Provide the source archive beside binaries. It contains the project files and
the pinned dependency sources under `dependency-sources/`. The regular builder
prepares complete build checkouts in `build-deps/`. Keep ROMs, local configuration, saves, signing
keys, compiler caches and diagnostic captures out of source and binary archives.

Use [release-notes.md](release-notes.md) as the release description and publish
`SHA256SUMS.txt` with the Windows ZIP, Linux tar.gz, Android APK/ZIP and Source
tar.gz. Keep the Android signing key private and use it for subsequent versions.
The build and packaging tools operate locally and do not upload or publish files.
