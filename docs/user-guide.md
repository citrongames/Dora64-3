# User guide

## ROM and first launch

Use the Japanese N3DJ revision 0 ROM, a 16 MiB big-endian `.z64` file.
Windows and Linux find a matching ROM beside the executable, regardless of
its filename. Android imports the selected ROM through the system picker.
ZIP, `.v64` and `.n64` files must be converted to the required `.z64` format first.

Extract desktop packages completely into a writable folder. Keep `assets`
and all bundled libraries beside the program. Settings and saves are stored
there; keep them when upgrading. Android uses its own application storage.

## Settings

Press **Esc** on desktop or tap the **gear** on Android. Close the menu with
Continue to return to the game. Reset game restarts the game in the same window.

| Tab | Settings |
| --- | --- |
| Game | Language, autosave, Modern/Original profile |
| Graphics | Frame rate, resolution, aspect ratio, HUD layout and filtering |
| Audio | Volume and sound settings |
| Controls | Player assignments, keyboard, mouse and gamepad bindings |
| Touch (Android) | On-screen controls and layout editing |
| Cheats | Optional gameplay assists |

The Modern profile enables the port's enhancements and uses Manual 60 FPS.
The Original profile selects the original frame rate, camera and presentation.
Languages are Japanese, English and Russian.

## Controls and multiplayer

Use the Controls tab to view or change bindings. Each of the four players
has a separate input assignment; gamepads and keyboard bindings can be mixed.
Mouse and right-stick camera controls apply where the modern camera is available.
Fixed-camera scenes, dialogue, cutscenes and minigames retain their intended camera.

On Android, the left stick moves the character and the on-screen buttons
provide the controller inputs. Drag in the camera area to move the modern
camera. The Touch editor lets you move and scale the controls; the layout
is saved automatically. Physical gamepads are also supported.

## Saves and upgrades

Autosave runs when returning to the stage-selection map. Keep the desktop
save files and settings when updating the executable or extracting a newer
package. On Android, install updates over the existing application using
the same signing key; uninstalling removes application data.

## Troubleshooting

- **ROM not found:** keep a matching `.z64` file beside the desktop executable,
  or choose it again in the Android picker. Check its size and SHA-256 in the README.
- **Missing DLL:** extract the complete Windows archive; do not copy only the EXE.
- **Graphics initialization fails:** update your graphics driver and confirm
  that the device supports the required backend. Android and Linux require Vulkan.
- **Linux loader or GLIBC error:** use a compatible distribution or build locally;
  the distributed 1.0.0 Linux binary requires glibc 2.43+.
- **Controls differ from your preference:** reassign them in Controls. On Android,
  check the Touch layout and connected gamepad assignment.
