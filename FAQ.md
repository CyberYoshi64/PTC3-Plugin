# Frequently asked questions

## My projects are gone when I'm using this plugin?

It's not gone. SmileBASIC is no longer looking in its original save data, but the `/PTC3PLG/savefs` folder.

Once the launcher for this plugin is released, this issue should be resolved. For now, have this guide on how to transfer the data yourself:

### How to transfer the save from SB3 to /PTC3PLG

- Config save data
  - Holds settings like editor colors and the pointer to the SmileButton program.
  1. Open FBI's main menu. Select "Titles".
  2. In the list, select "SmileBASIC" or "Petit Computer 3"
  3. Select "Browse save data".
  4. Select "\<current directory>", then "Copy all contents".
  5. Go all the way back to the main menu and select "SD".
  6. Select "PTC3PLG", then "savefs".
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

- Check, if you're running [the latest version of Luma 3DS with plugin loader support](https://github.com/LumaTeam/Luma3DS/releases/latest).
- Are you on the TOP MENU? The menu is disabled in BASIC to prevent the screenshot feature from triggering when you don't want it to.
- Have you remapped the hotkey for the in-app menu at some point but forgot?
  - Delete the file `/PTC3PLG/resources/CTRPFData.bin`. This will reset the button to SELECT.
