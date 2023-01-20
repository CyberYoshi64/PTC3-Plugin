# Frequently asked questions

## My projects are gone when I'm using this plugin?

It's not gone. SmileBASIC is no longer looking in its save data, but the `/PTC3PLG/config/` and `/PTC3PLG/savefs` folders.

Once the launcher for this plugin is released, this issue should be resolved. For now, have this text guide:

### How to transfer the save from SB3 to /PTC3PLG

- Config save data
  - Holds settings like editor colors and the pointer to the SmileButton program.
  1. Open FBI's main menu. Select "Titles".
  2. In the list, select "SmileBASIC" or "Petit Computer 3"
  3. Select "Browse save data".
  4. Select "\<current directory>", then "Copy all contents".
  5. Go all the way back to the main menu and select "SD".
  6. Select "PTC3PLG", then "config".
      - If the folders don't exist, create them, then enter them.
  7. Select "\<current directory>", then "Paste".
  8. Select "Yes" and wait for the transfer to finish.
- Project save data (extData)
  - Contains all your projects.
  1. Open FBI's main menu. Select "Ext Save Data".
  2. In the list, select "SmileBASIC" or "Petit Computer 3"
  3. Select "Browse User Save Data".
  4. Select "\<current directory>", then "Copy all contents".
  5. Go all the way back to the main menu and select "SD".
  6. Select "PTC3PLG", then "savefs".
      - If the folders don't exist, create them, then enter them.
  7. Select "\<current directory>", then "Paste".
  8. Select "Yes" and wait for the transfer to finish.

## The plugin doesn't work / I cannot open the in-app menu.

- Check, if you're running [the latest version of Luma 3DS with plugin loader support](https://github.com/Nanquitas/Luma3DS/releases/latest).
- Have you remapped the hotkey for the in-app menu at some point but forgot?
  - Delete the file `/PTC3PLG/resources/CTRPFData.bin`. This will reset the button to SELECT.

## The hotkey for the in-app menu is inconvenient. I keep making screenshots when opening the menu...

The default hotkey is set by CTRPluginFramework and the fine folks there figured that the SELECT button was the best button to use as a default.

To change the hotkey, open the in-app menu, then tap "Tools", select "Settings", then "Change menu hotkeys". There you can set the hotkey to your liking.

## I'm on New3DS but the C Stick doesn't work / SmileBASIC doesn't detect ZL/ZR, when using the plugin.

This is a bug in CTRPluginFramework that needs to be fixed. CTRPluginFramework can use ZL/ZR buttons, as of now, but fails to pass the buttons through to SmileBASIC.

## [Citra] The in-app menu is making horrible grinding noises; it like freezes the audio

This is an emulation inaccuracy caused by Citra, whereas the sound keeps trying to play, even if it shouldn't. I cannot find and hook the related function in SmileBASIC's code, so I'm sorry for this issue.

Note that any other issue that is Citra-specific and does not occur on 3DS is to be reported to the Citra Team, instead of me, m'kay?