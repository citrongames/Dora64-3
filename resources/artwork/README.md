# Dora64-3 application icon

The owner selected concept 1 (the abducted town) on 2026-10-10. The original
PNG was generated with the built-in image_gen tool and is preserved unchanged
as dora64-3-icon.png. Its text is ド, 64 and 3. See icon-prompt.json for the prompt.
The motif refers to the town being transported to another planet; it is original
concept artwork, not an extracted ROM asset or an exact in-game town model.
Story reference: https://gamefaqs.gamespot.com/n64/577493-doraemon-3-nobita-no-machi-sos/faqs/79013

Run `python3 tools/export_app_icon.py` with Pillow to regenerate the checked-in
exports. This only resizes/converts formats; it does not redraw the selected art.
Normal builds need neither Pillow nor an image-generation service.

Windows ICO includes 16, 32, 48, 64, 128, 256 pixels. PNGs additionally include512.
The runtime embeds the256 PNG through RT64 file_to_c and sets the SDL window icon;
Windows also embeds the ICO as resource group1 for Explorer/shortcuts. Linux
builds include desktop registration files and the PNG sizes. Run the included
install-desktop.py manually to register a Linux build; building never installs it.
The existing Linux executable name dora3_startup is retained.

This follows Dora64's desktop integration, without modifying the donor project.
Gameplay, Original/Native/Manual FPS paths and renderer behaviour are unchanged.
No tests, CPU checks, game/emulator/renderer/GPU launches. Visual verification is
by the user; commit/push remains pending explicit authorization after that check.

## Acceptance on 2026-10-10

After test104 was delivered, the owner explicitly authorized commit/push of
this icon integration to main. This supersedes the pending Git status above.
No additional build or automated/gameplay execution; Linux remains unbuilt.
