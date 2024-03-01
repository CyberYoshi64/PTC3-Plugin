# 0.0.6-4 (2024/3/1, 23:22)

- Replace `FwkSettings.WaitTimeToBoot` with `StallProcess()`
    - Using a hook into the game's `main()` function to start the main plugin menu
- Add structs `appletFlag`, `ptcScreen` and some associated member structs
- Add `PrintCharFunc` definition to print characters to the console
