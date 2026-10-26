# Batman: Arkham Knight for iPad and Mac

An unofficial, fan-made port of **Batman: Arkham Knight** that runs on iPad and Apple silicon Macs.

> This project is not affiliated with or endorsed by Warner Bros. Games, Rocksteady Studios or DC. It contains no game files and offers no ready-to-play download: you need your own copy of the game, from which you build the app yourself.

## Features

- **60 fps**, twice the original's 30.
- **A sharper picture:** a **render scale** up to 100 % (the original holds the 3D scene at 75 %), the engine's own **anti-aliasing** (SMAA, off in the original) and **texture filtering** up to 16×.
- **Resolution** 1920 × 1080 or 1280 × 720.
- **Sound**, **saves** and **game controllers**; on the Mac the keyboard too, on iPad on-screen controls.
- A **settings screen** before the game, on every device, that remembers your choices.

## Requirements

- **iPad:** an iPad with an M-series chip on iPadOS 26 or later; tested on an iPad Pro with M4. About 28 GB free once installed, and more while it installs. iPhone is not tested yet.
- **Mac:** a Mac with Apple silicon (tested on macOS 26). Installing on an iPad also needs about 30 GB free on the Mac's internal disk, where AltServer unpacks the app.
- **Your own copy of the game**, with update 1.0.5.

## Getting it

1. Build the app from your copy of the game: [docs/nxnative.md](docs/nxnative.md) has the steps (*Pack an IPA*), and [docs/development.md](docs/development.md#a-mac-only-build-for-development) the Mac version, which comes with its own launcher app. The iPad app is about 27 GB.
2. **iPad:** install the IPA with [AltServer](https://altstore.io) 1.8 or later (*Sideload .ipa*: hold ⌥ Option and click its menu-bar icon), over a cable. It takes about 30 minutes; keep the iPad unlocked until it is done.
3. **Mac:** open the launcher app that the build puts in your game folder and press **Play** (keep the folder: the game runs from it).

## Playing

- **Settings:** render scale, anti-aliasing, texture filtering, resolution, game language, full screen (Mac), rumble, a frame-rate counter and a performance overlay. For the sharpest picture: render scale 100 %, anti-aliasing on, texture filtering 16×. If the iPad cannot hold 60 fps, lower the render scale.
- **Controls:** a game controller works everywhere. On the Mac: WASD to move, arrow keys for the D-pad, J / K / U / I for A / B / X / Y, Q / E for L / R, Z / C for ZL / ZR, Return for + and Tab for −. On iPad, the on-screen controls hide when a controller connects.
- **The first start** takes a minute or two to reach the language screen; the story movies play at their own 30 fps.
- **No sound on iPad?** The app follows the device's silent mode: turn silent mode off.
- **Saves and logs** are in the Files app, under the app's name. When something goes wrong, the log is `log.txt` there.
- **An iPad app built before 60 fps was added** gets it from a text file named `env.txt` in that same folder, with the line `DM_PRESENT_INTERVAL=display`.

## Status

Tested on the Mac from the start of the game through the menus into Gotham, and on an iPad Pro (M4) at 60 fps. Known gaps: only update 1.0.5 was tested, and only the start of the game; graphics compile the first time each scene appears, so new scenes can stutter once; textures stay at the original's size (the game has no larger ones). Details for developers are in the game's note, [dm/games/0100ACD0163D0000/README.md](dm/games/0100ACD0163D0000/README.md).

## Credits and licence

Built on [nxnative](https://github.com/seedds/nxnative) and the projects it credits ([docs/nxnative.md](docs/nxnative.md#acknowledgements)). The code is under the MIT licence ([LICENSE](LICENSE)). Batman and all related names are trademarks of DC; Batman: Arkham Knight is a game by Rocksteady Studios, published by Warner Bros. Games.

## Support me

If you want to get cool ports or to see many cool things I develop I will appreciate some support from the community to keep going :) 

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/N0T729IA98)
