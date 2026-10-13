# Android integration

Dora64-3 uses SDL2 Activity and `libDora64Three.so` on Android ARM64.
The minimum platform is Android7.0/API24 with Vulkan1.0. The manifest declares
both `android:appCategory="game"` and `android:isGame="true"`.

The application has its own ID, icon and storage. A system document picker
imports and verifies the Japanese N3DJ revision0 ROM. Localization and artwork
are bundled as Android assets. Settings, saves and touch layout are stored
inside this application's data.

The mobile menu uses larger controls, independently scrollable tab content
and a persistent footer. The Touch tab edits button positions and sizes.
Touch input preserves short contacts, owns each finger independently and
cancels contacts when the application context or focus changes. Physical
controllers share the port's four-player assignment system.

Rendering uses the pinned RT64/Plume Android compatibility paths. Audio and
native frame production follow SDL lifecycle events; the NDK27 implementation
uses std::thread and the same wake/stop protocol as the desktop runtime.
All game-specific timing, camera, animation and resource adapters are shared
with the desktop port where applicable.

Release builds use CMake Release, disable debugging, diagnostic journals and
profiling, and strip native symbols during APK packaging. The project version
comes from VERSION. A separate private signing configuration signs release APKs.

See [Android setup](../android/README.md) and [build instructions](building.md#android).
