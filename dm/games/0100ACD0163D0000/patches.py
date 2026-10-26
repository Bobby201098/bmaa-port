# Batman: Arkham Knight (0100ACD0163D0000): the rewriter's patches of this game's own builds. dm/tools/patch_nso.py loads every profile's patches.py and applies those of a module's NSO build ID;
# CUTS, OBS and FRAME_STEP as described there ("Registered patches"). Patches of Nintendo SDK module builds are in dm/tools/sdk_patches.py
CUTS = {
  # Batman: Arkham Knight 1.0.5 (v327680) main: the engine's allocator (Unreal Engine 3; its own binned allocator, vtable main+0x329a408) is a static object at main+0x51818c0, constructed
  # inline under a C++ static guard by the function that makes GMalloc (main+0x192eb30, GMalloc = main+0x483f770). The construction reserves 1 GB (main+0x192eee8) and 37 GB of address space
  # (main+0x192fe50: 74 pools of 512 MB, pool index = offset >> 29, base also at main+0x3fd1088), which only the Mac can give: an iPad has about 5 GB without the extended virtual addressing
  # entitlement. The construction is skipped: the guard is released at once (main+0x192fea4), so the allocator is never built and its atexit destructor never registered. The host has put its
  # own allocator's vtable in the object (dm/host/hle_uemalloc.c, layout UE3): every reference to the pool base and the 1 GB region table is inside the allocator's own functions
  # (main+0xb2b37c-0xb2e388), which nothing outside its vtable calls
  'feae83dc9e9fb34c77d0a62f1fa9ae9f': [
    (0x192ec14, '68cb0090', 'a4040014', 'GMalloc creation: adrp x8 (start of the allocator construction, 38 GB of address space) → b 0x192fea4 (the guard release); the host allocator is in the object already (hle_uemalloc.c)'),
  ],
}
