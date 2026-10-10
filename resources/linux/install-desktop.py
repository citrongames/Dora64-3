#!/usr/bin/env python3
"""Register this portable Linux build in the current user's applications menu."""

import os
from pathlib import Path
import shutil
import subprocess


def desktop_string(value):
    return (str(value).replace("\\", "\\\\").replace("\n", "\\n")
            .replace("\r", "\\r").replace("\t", "\\t"))


def exec_argument(value):
    # Desktop Entry Exec has quoting rules in addition to string-value escapes.
    escaped = str(value).replace("\\", "\\\\")
    for character in ('"', '`', '$'):
        escaped = escaped.replace(character, "\\" + character)
    return desktop_string('"' + escaped.replace('%', '%%') + '"')


def main():
    game_dir = Path(__file__).resolve().parent
    executable = game_dir / 'dora3_startup'
    icons = game_dir / 'assets/icons'
    sizes = (16, 32, 48, 64, 128, 256, 512)
    required = [executable, game_dir / 'dora64-3.desktop']
    required += [icons / f'dora64-3_{size}.png' for size in sizes]
    for path in required:
        if not path.is_file():
            raise SystemExit(f'Missing build file: {path}')
    if not os.access(executable, os.X_OK):
        raise SystemExit(f'Game is not executable: {executable}')

    configured = os.environ.get('XDG_DATA_HOME', '')
    data_home = Path(configured) if configured else Path.home() / '.local/share'
    if not data_home.is_absolute():
        raise SystemExit('XDG_DATA_HOME must be an absolute path')
    applications = data_home / 'applications'
    applications.mkdir(parents=True, exist_ok=True)
    theme = data_home / 'icons/hicolor'
    for size in sizes:
        destination = theme / f'{size}x{size}/apps/dora64-3.png'
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(icons / f'dora64-3_{size}.png', destination)

    lines = (game_dir / 'dora64-3.desktop').read_text().splitlines()
    lines = ['Exec=' + exec_argument(executable) if line.startswith('Exec=')
             else line for line in lines]
    lines.append('Path=' + desktop_string(game_dir))
    launcher = applications / 'dora64-3.desktop'
    launcher.write_text('\n'.join(lines) + '\n')
    for command in (['update-desktop-database', str(applications)],
                    ['gtk-update-icon-cache', '-f', '-t', str(theme)]):
        if shutil.which(command[0]):
            subprocess.run(command, check=False)
    print(f'Installed {launcher}')
    print('Run this installer again if you move the game directory.')


if __name__ == '__main__':
    main()
