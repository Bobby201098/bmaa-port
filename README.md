# Batman: Arkham Asylum for iPhone, iPad and Mac

An unofficial, fan-made port of **Batman: Arkham Asylum** that runs on iPhone, iPad and Apple silicon Macs.

> This project is not affiliated with or endorsed by Warner Bros. Games, Rocksteady Studios or DC. It contains no game files and offers no ready-to-play download: you need your own copy of the game, from which you build the app yourself.

## Features

- **60 fps**, twice the original's 30.
- **Resolution** 1920 × 1080 or 1280 × 720, and a **render scale** that trades sharpness for speed.
- **Sound**, **saves** and **game controllers**; on the Mac the keyboard too, on iPhone and iPad on-screen controls.
- A **settings screen** before the game, on every device, that remembers your choices.

## Requirements

- **iPhone or iPad:** an iPhone 15 Pro or newer, or an iPad with an M1 chip or later, on iOS / iPadOS 26 or later. About 17 GB free while installing, 8.4 GB once installed.
- **Mac:** a Mac with Apple silicon (tested on macOS 26).
- **Your own copy of the game.**

## Getting it

1. Build the app from your copy of the game: [docs/nxnative.md](docs/nxnative.md) has the steps (*Pack an IPA*), and [docs/development.md](docs/development.md#a-mac-only-build-for-development) the Mac version, which comes with its own launcher app.
2. **iPhone / iPad:** install the IPA with [AltServer](https://altstore.io)'s *Sideload .ipa* (hold ⌥ Option and click its menu-bar icon), over a cable. It takes about 15 minutes. Sideloadly could not install this app.
3. **Mac:** open the launcher app that the build puts in your game folder and press **Play** (keep the folder: the game runs from it).

## Playing

- **Settings:** frame rate, resolution, render scale, game language, full screen (Mac), rumble and a performance overlay.
- **Controls:** a game controller works everywhere. On the Mac: WASD to move, arrow keys for the D-pad, J / K / U / I for A / B / X / Y, Q / E for L / R, Z / C for ZL / ZR, Return for + and Tab for −. On iPhone and iPad, the on-screen controls hide when a controller connects.
- **No sound on iPhone or iPad?** The app follows the device's silent mode: turn silent mode off.
- **Saves and logs** are in the Files app, under the app's name. When something goes wrong, the log is `log.txt` there.

## Status

Tested from the start of the game into its first area, on the Mac and on iOS. Known gaps: only the original release (1.0.0) was tested, and graphics compile the first time each scene appears, so new scenes can stutter once.

## Credits and licence

Built on [nxnative](https://github.com/seedds/nxnative) and the projects it credits ([docs/nxnative.md](docs/nxnative.md#acknowledgements)). The code is under the MIT licence ([LICENSE](LICENSE)). Batman and all related names are trademarks of DC; Batman: Arkham Asylum is a game by Rocksteady Studios, published by Warner Bros. Games.

##Support me
If you want to get cool ports or to see many cool things I develop I will appreciate some support from the community to keep going :) 
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/N0T729IA98)
