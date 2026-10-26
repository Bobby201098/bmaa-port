#!/usr/bin/env python3
# The Mac launcher for one game (Launcher.swift beside this file): <out>/<name>.app, compiled with swiftc (no Xcode project), with the game's icon, its settings for the Mac
# (dm/games/settings.py <id> … mac → Resources/launcher.json) and how to run it (Resources/launch.json: the Mac host, the rewritten modules, the run folder, the log and the base
# environment; the NSP script passes the command it prints for --platform mac). The app refers to the game's files where they are; it holds none of them.
# Usage: make_launcher.py <out dir> <name> <game id> <display version> <rewritten modules dir> <control.nacp> <icon.jpg> <launch.json>   (DM_GAMES_DIR as for game.py; DM_LANG = the
#        language the launcher starts with)
import json, os, plistlib, re, shutil, subprocess, sys, tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
MIN_MACOS = '14.0'


def icns(jpg, out):   # the game's square icon (control data, 256x256) as an .icns: each size iconutil wants, scaled by sips
    with tempfile.TemporaryDirectory() as t:
        s = Path(t) / 'AppIcon.iconset'; s.mkdir()
        for px in (16, 32, 128, 256, 512):
            for k in (1, 2):
                n = f'icon_{px}x{px}{"@2x" if k == 2 else ""}.png'
                subprocess.run(['sips', '-s', 'format', 'png', '-z', str(px * k), str(px * k), str(jpg), '--out', str(s / n)], check=True, capture_output=True)
        subprocess.run(['iconutil', '-c', 'icns', str(s), '-o', str(out)], check=True, capture_output=True)


def build(out, name, gid, version, patched, control, icon, launch):
    app = Path(out) / f'{name}.app'; tmp = app.with_name(app.name + '.partial')
    shutil.rmtree(tmp, ignore_errors=True)
    (tmp / 'Contents/MacOS').mkdir(parents=True); (tmp / 'Contents/Resources').mkdir(parents=True)
    exe = re.sub(r'[^A-Za-z0-9]', '', name) or 'Launcher'
    subprocess.run(['xcrun', 'swiftc', '-O', '-parse-as-library', '-target', f'arm64-apple-macos{MIN_MACOS}', str(HERE / 'Launcher.swift'),
                    '-o', str(tmp / 'Contents/MacOS' / exe)], check=True)
    plistlib.dump({'CFBundleName': name, 'CFBundleDisplayName': name, 'CFBundleExecutable': exe, 'CFBundleIdentifier': f'org.nxnative.mac.{gid.lower()}',
                   'CFBundlePackageType': 'APPL', 'CFBundleShortVersionString': version, 'CFBundleVersion': version, 'CFBundleIconFile': 'AppIcon',
                   'LSMinimumSystemVersion': MIN_MACOS, 'NSHighResolutionCapable': True, 'LSApplicationCategoryType': 'public.app-category.games'},
                  open(tmp / 'Contents/Info.plist', 'wb'))
    if Path(icon).is_file(): icns(icon, tmp / 'Contents/Resources/AppIcon.icns')
    subprocess.run([sys.executable, str(HERE.parent / 'games/settings.py'), gid, str(patched), str(control), str(tmp / 'Contents/Resources/launcher.json'), 'mac'],
                   check=True, capture_output=True, text=True)
    json.loads(Path(launch).read_text()); shutil.copyfile(launch, tmp / 'Contents/Resources/launch.json')
    shutil.rmtree(app, ignore_errors=True); tmp.rename(app)
    return app


if __name__ == '__main__':
    if len(sys.argv) != 9: sys.exit('usage: make_launcher.py <out dir> <name> <game id> <display version> <rewritten modules dir> <control.nacp> <icon.jpg> <launch.json>')
    print(build(*sys.argv[1:]))
