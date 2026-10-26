# Batman: Arkham Knight (`0100ACD0163D0000`)

Update 1.0.5 (v327680), SDK 14.3.0; the base game (v0) is a 7 MB stub and the update holds the game (26.9 GB). Rocksteady's Unreal Engine 3 (package version 864, licensee 0x80e3) on NVN, with Wwise for the sound (hardware Opus, which the host gives the SDK's software decoder), CRI Sofdec for the movies (`.usm`) and Scaleform for the menus. Modules `main` and `sdk`; the `sdk` is Batman: Arkham Asylum's build.

Status: plays on the Mac, in a window with sound and keyboard input, from boot through the title, the save select and the main menu into gameplay in Gotham (the player played it). 60 fps on the Mac (30 on the Switch; see [Frame rate](#frame-rate)). The iPad (M4): the first build stopped as its menus loaded, out of address space; the build with the 2 GB block runs past that point at 30 fps (see [Memory](#memory)).

## What it needs

| Setting | Where | Why |
|---|---|---|
| The allocator's construction skipped; the host's allocator in its object | [`patches.py`](patches.py), `hle_uemalloc.c` | it would reserve 37 GB of address space, which an iPad cannot give; see [Memory](#memory) |
| A 2 GB block: `mem_reserve 0x80000000`, `mem_alias 0x58000000` | [`game.tsv`](game.tsv) | once the allocator is the host's the game maps almost nothing in the block, and the heap outside it needs the address space; the same on the Mac and on iOS; see [Memory](#memory) |
| `tex_flush_upload on`, `tex_dirty_upload on` | `game.tsv` | guessed from the loose Unreal Engine 3 packages, as for Batman: Arkham Asylum; not checked yet (no 3D scene reached) |
| `sys_resource 0x2000000` | `game.tsv` | the NPDM declares 32 MB of system resource: virtual address memory |
| `ue3_ini on` | `game.tsv`, `hle_ue3ini.c` | the launchers' Render scale, Anti-aliasing and Texture filtering set the engine's own settings; see [Picture](#picture) |
| `present_interval display` | `game.tsv` | the window's present interval 2 is the only thing that holds the game at 30; see [Frame rate](#frame-rate) |

One `patches.py` cut (the allocator) and no `sdk_patches.py` cut. The profile ships no shader tables: the NSP script makes them from the game files in a profile derived from this one, beside the build (`derive_profile`), as for Batman: Arkham Asylum.

## Files and shaders

- 4,092 cooked packages (`BmGame/CookedSwitch/*.xxx`) in Oodle Mermaid chunks, 12 GB; 10 texture file caches (`Chunk0`–`7.tfc`, `Textures.tfc`, `Heightmaps.tfc`); 1,030 Sofdec movies (`BmGame/Movies/*.usm`, 7.6 GB); voice packs per language (`<language>/*.wad`, about 0.8 GB each for 8 languages, and `SFX/BNK.wad`). The RomFS is 26.7 GB.
- 22,001 shaders (2,614 vertex, 19,261 pixel, 124 compute, 2 geometry) from `CombinedShaderCache-Switch.xxx`, the packages' shader caches and `GlobalShaderCache-Switch.bin`, read by `shader_split.py` (`ue3_shaders`): [Unreal Engine 3 shader caches in Oodle-compressed packages](../../../docs/reference/shaders.md#unreal-engine-3-shader-caches-in-oodle-compressed-packages). Package version 864 adds four summary fields and numbers the shader stages with hull, domain, geometry and compute shaders; both are read by version.
- **Resolved: 40 of the first 94 shaders were not in the shader map.** The game's shader loader (`main+0xb7948c`) shares control sections that differ only in two 24-byte IDs, so a shader runs with the first variant's IDs; the keys now leave those bytes out (24,721 keys → 22,001): [Control sections shared by a key that leaves out two IDs](../../../docs/reference/shaders.md#control-sections-shared-by-a-key-that-leaves-out-two-ids).
- The Mac shader build took about 1 h 30 min on the SD card the files live on (an M3 Pro): the compile itself about an hour, the rest writing and removing small files.

## Memory

- **Resolved: a write fault at `main+0xb2dae8` in `nninitStartup`** after `err::ShowError` 0x1803 and 0xe03: the allocator's 37 GB address region did not fit the provisional 4.375 GB alias region, and it committed pages at a base of 0: [Unreal Engine 3's allocator over 37 GB of address space](../../../docs/reference/memory.md#unreal-engine-3s-allocator-over-37-gb-of-address-space). By its first frames the game has used up to +43 GB of the alias region, about 1.5 GB of it committed.
- **Resolved for iOS:** the allocator's construction is skipped and the host's allocator runs in its object (layout `UE3` in `hle_uemalloc.c`): in gameplay in Gotham on the Mac the host heap held 2.25 GB and the process 3.28 GB.
- **Resolved: the first iPad build (M4) stopped with `FATAL Unreal heap: out of memory (8388608 bytes, alignment 2097152, 1.79 GB in use)`** about 50 s in, as the menus loaded, in both of its runs. The host heap lies outside the guest block, and Engage's 5 GB block, the heap and 1 GB of Metal allocations used up the address space iOS gives the app: [The host's Unreal heap shares the address space with the guest block](../../../docs/reference/memory.md#the-hosts-unreal-heap-shares-the-address-space-with-the-guest-block). With the host allocator the game maps one page of the alias region and 14 MB of thread stacks through the title (`DM_SVC=1`), and nothing past +256 MB of it into Gotham, so the block is 2 GB now.

## Rendering

- **Resolved: the language screen drew pale shapes and no text** (the health warning only its headline). Scaleform binds its vertex streams at addresses that are 2 mod 4, which Metal reads from the aligned word below; the batched text shader's `RGBA8UI` batch index read the byte before it, every glyph got a transform past the uniform buffer and collapsed: [Vertex streams at addresses that are not multiples of 4](../../../docs/reference/rendering.md#vertex-streams-at-addresses-that-are-not-multiples-of-4). The glyph cache itself (`R8`, linear, `(1, 1, 1, R)`) was right throughout.
- One `[RED] texture type mismatch: shader declares a cube, texture is 2D` at start; not looked into.

## Picture

- The Switch settings (in `Coalesced_INT.bin`: `Switch-BmSystemSettings.ini [SystemSettings]`, `Switch-BmEngine.ini [DynamicResolution]`) lock the 3D scene to 75 % of the screen (`MinScreenPercentage 75`, `bLockedToMin True`, `ScreenPercentage 75`), leave the engine's SMAA off (`bAllowPostprocessMLAA False`; the higher quality buckets turn it on, and its shaders are in the global shader cache) and filter 4× (`MaxAnisotropy 4`). With `ue3_ini on` the launchers' Render scale, Anti-aliasing and Texture filtering set these (`hle_ue3ini.c`); the player found the picture at 100 %, SMAA and 16× right.
- Texture groups are capped at `MaxLODSize 256` (world, characters, weapons). **Do not raise it:** at 4096 the opening cutscene's faces came out scrambled; the cook holds no larger mips.

## Frame rate

- **Current: 60 fps** (30 on the Switch) with `present_interval display`: on the Mac, frame time p50 16.7 / p95 17.1 ms in the menus and p50 16.7 / p95 17.5 ms in gameplay in Gotham at the game's 75 % (GPU p50 5.9 ms); the Sofdec movies keep their own 30. The window is built with present interval 2, and that is the only 30 fps limit: `UGameEngine::GetMaxTickRate` (`main+0x1283dd4`) returns the float at `[GEngine]+0x884` while the vsync flag at `main+0x5117244` is set (cleared by `-novsync` on the command line or the `UNCAPFPS` command), and that float is 62 (`MaxSmoothedFrameRate` of `[Engine.Engine]` in `Switch-BmEngine.ini`; +0x888 holds the minimum, 24). `appUpdateTimeAndHandleMaxTickRate` (`main+0x1932960`; `GEngine` is the pointer at `main+0x511a150`) steps the game by the time that really passed, since `GUseFixedTimeStep` (`main+0x483f800`) is 0: at 60 fps `GDeltaTime` is 0.0167 s and `GCurrentTime` advanced 10.000 s in 10 s. Unlike Batman: Arkham Asylum ([Unreal Engine 3's tick limit and a window built with interval 2](../../../docs/reference/frame-rate.md#unreal-engine-3s-tick-limit-and-a-window-built-with-interval-2)), no row writes the tick limit.
- The iPad build made before the key was added takes it from `Documents/env.txt` (`DM_PRESENT_INTERVAL=display`).
- Before 60 fps, at 30 as on the Switch: in a ten-minute session on the Mac (SD card, render scale 100 %): frame time p50 33.3 ms, GPU p50 9 ms; of 676 long frames, 295 were shader and pipeline builds on first use (19.9 s lost), 213 the game's own CPU work or file reads (9.9 s), 165 the GPU (1.0 s). The first kind goes as `pso_user.tsv` fills (3,141 lines after that session; the next start prewarmed 1,249 in 2 s); there is no `pso_manifest.tsv` yet. There is no 30 fps choice in the launchers yet (no `fps_target`).

## Mac routes

- The language screen comes about 90 s after the start on the SD card the files live on; A (J on the keyboard) picks English and shows the health warning.

## Found on this game

General changes first needed here; each applies to every game:

- **Unreal Engine 3 file version 864**: four more summary fields (`uepak.ue3_summary`) and the stage numbering with hull, domain, geometry and compute shaders (`shader_split.py` `UE3_FREQ_DX11`): [Unreal Engine 3 shader caches in Oodle-compressed packages](../../../docs/reference/shaders.md#unreal-engine-3-shader-caches-in-oodle-compressed-packages).
- **A key mask for control sections an engine shares** (`shader_split.py` `key_mask`, the map's `# key mask:` line, `hle_nvn.c` `keys_hash`): [Control sections shared by a key that leaves out two IDs](../../../docs/reference/shaders.md#control-sections-shared-by-a-key-that-leaves-out-two-ids).
- **A 64 GB alias region guessed for Unreal Engine 3 with virtual address memory** (`profile_guess.py`): [Unreal Engine 3's allocator over 37 GB of address space](../../../docs/reference/memory.md#unreal-engine-3s-allocator-over-37-gb-of-address-space).
- **Unreal Engine 3's settings from the launchers** (profile key `ue3_ini`, `hle_ue3ini.c`; the general Texture filtering setting `DM_TEX_FILTER` in `nvn_mtl.m`): see [Picture](#picture) and the key in [dm/games/README.md](../README.md).
- **Unreal Engine 3's allocator replaced by the host's** (`hle_uemalloc.c` layout `UE3`, a branch cut with its own expected word): [Unreal Engine 3's allocator over 37 GB of address space](../../../docs/reference/memory.md#unreal-engine-3s-allocator-over-37-gb-of-address-space).
- **Vertex streams at unaligned addresses copied to aligned memory** (`nvn_mtl.m` `vb_realign`): [Vertex streams at addresses that are not multiples of 4](../../../docs/reference/rendering.md#vertex-streams-at-addresses-that-are-not-multiples-of-4).
- **60 fps from the present interval alone**, the engine's own limit being 62: [Unreal Engine 3's tick limit and a window built with interval 2](../../../docs/reference/frame-rate.md#unreal-engine-3s-tick-limit-and-a-window-built-with-interval-2).
- **The guest block sized beside the host's Unreal heap** on iOS: [The host's Unreal heap shares the address space with the guest block](../../../docs/reference/memory.md#the-hosts-unreal-heap-shares-the-address-space-with-the-guest-block).

## Open

- Past the health warning: the title, a save, gameplay; the 3D scene and whether it needs `tex_flush_upload` / `tex_dirty_upload`.
- 60 fps on the iPad: not yet measured there (the M4's GPU at render scale 100 %, heat over a long session). A 30 fps choice for battery would need `fps_target` and a row writing `[[main+0x511a150]]+0x884`.
- One windowed run crashed early at `main+0xb2b4f0` (the allocator, a read at `0x39431234`); the retry and every headless run did not.
- The iPad: the first IPA (27 GB; the voice packs of the languages not played are 6.4 GB of it) played the intro movies at 30 fps and stopped at the menus (see [Memory](#memory)); the build with the 2 GB block is not yet run on a device.
