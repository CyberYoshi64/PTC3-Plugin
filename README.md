# PTC3-Plugin

This [CTRPF](https://gitlab.com/thepixellizeross/ctrpluginframework/) plugin is for use with SmileBASIC 3.

### Supported versions

- EUR 3.6.0
- USA 3.6.0
- JPN 3.6.3

### Features

Currently available:

- Redirect romfs/savefs to SD Card (see "Setup")
- Dark mode (WIP) (See "Setup")
- Version spoofer
- Server changer (for use with [SBServer](https://github.com/trinitro21/sbserver))

To do:

- IDK, it's a lot... lol

### Preliminary builds

A sample of v0.0.5 can be found in [here](https://cyberyoshi64.github.io/assets/data/CY64-CYX-v0.0.5.zip).

## Other sources

- [CTGP-7 Open Source Project](https://github.com/PabloMK7/CTGP-7_Open_Source)
- [Vapecord-ACNL-Plugin](https://github.com/RedShyGuy/Vapecord-ACNL-Plugin)


## Building

Requirements:

- devkitPro with 3DS toolchains installed
- [bannertool](https://github.com/Steveice10/bannertool/releases/latest)
- [makerom](https://github.com/3DSGuy/Project_CTR/releases)
- [3gxtool v1.1+](https://gitlab.com/thepixellizeross/3gxtool/-/releases)

Once you got these requirements, just `make`.

## Setup

The plugin does not have an associated launcher at the moment.
For now, it has to be placed in `/luma/plugins/[TID]/`, whereas TID is:

- JPN: 0004000000117200
- USA: 000400000016DE00
- EUR: 00040000001A1C00

### The PTC3PLG folder

This folder contains romfs overlay and savefs for the plugin.

If it's not created, it will make the folders for you, however, it will look like your save data is gone. Don't worry, as it's not gone! See `FAQ.md` for details on how to transfer your save.

**Any changes you perform in this folder will not affect SmileBASIC, as well as your main save data.**

### Dark Mode (WIP)

This option changes the color scheme of the application to suit dark themes (through SysUI.GRP and SysBASIC.GRP).

It's not a setting in the plugin, but a flag file in `/PTC3PLG/config/darkPalette.flag`

**This setting only changes text colors.**