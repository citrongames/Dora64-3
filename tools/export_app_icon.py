"""Export the approved artwork to desktop icon formats (Pillow; not a build dependency)."""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]

def main():
    destination = ROOT / 'assets/icons'
    destination.mkdir(parents=True, exist_ok=True)
    with Image.open(ROOT / 'resources/artwork/dora64-3-icon.png') as source:
        image = source.convert('RGBA')
        if image.width != image.height:
            raise ValueError('Approved icon must be square')
        sizes = (16, 32, 48, 64, 128, 256, 512)
        for size in sizes:
            image.resize((size, size), Image.Resampling.LANCZOS).save(
                destination / f'dora64-3_{size}.png')
        image.save(destination / 'dora64-3.ico', sizes=[(s, s) for s in sizes if s <= 256])

if __name__ == '__main__':
    main()
