"""Build the PubWell splash screens (light and dark) for the 1_7_0 icon set.

Same 490 x 454 canvas as the Scribus splash it replaces. The card keeps the
printer's crop marks, and the bottom right of the card stays clear because
ui/splash.cpp draws the version number and loading messages there.

The wordmark is cut from concept-b-chatgpt.png (the chosen concept), so it
matches the approved artwork exactly. The tagline uses Segoe UI from Windows.

Outputs: out/pubwell_splash_light.png, out/pubwell_splash_dark.png
Run: python make_splash.py
"""
import os
from PIL import Image, ImageDraw, ImageFont

from make_icons import HERE, OUT, PLUM, INK, render

W, H = 490, 454
CARD = (15, 88, 475, 408)           # same card as the Scribus splash
WORDMARK_BOX = (744, 298, 1433, 451)  # "PubWell" in the concept image
FONT = r"C:\Windows\Fonts\segoeui.ttf"
FONT_SEMIBOLD = r"C:\Windows\Fonts\seguisb.ttf"

THEMES = {
    "light": {"card": (247, 243, 237), "ink": INK, "sub": (96, 99, 104), "marks": (40, 40, 40), "line": (222, 212, 206)},
    "dark": {"card": (42, 45, 49), "ink": (250, 250, 250), "sub": (190, 192, 196), "marks": (200, 200, 200), "line": (66, 66, 72)},
}


def wordmark(height, color):
    """Wordmark from the concept image as a colored, transparent image."""
    src = Image.open(os.path.join(HERE, "concept-b-chatgpt.png")).convert("L").crop(WORDMARK_BOX)
    # Ink is about 44, paper about 252: map that range to alpha.
    alpha = src.point(lambda v: max(0, min(255, round((252 - v) * 255 / (252 - 44)))))
    width = round(alpha.width * height / alpha.height)
    alpha = alpha.resize((width, height), Image.LANCZOS)
    out = Image.new("RGBA", alpha.size, color + (0,))
    out.putalpha(alpha)
    return out


def crop_marks(d, color):
    x0, y0, x1, y1 = CARD
    gap, length = 4, 12
    for x, y, sx, sy in [(x0, y0, -1, -1), (x1, y0, 1, -1), (x0, y1, -1, 1), (x1, y1, 1, 1)]:
        d.line([(x + sx * gap, y), (x + sx * (gap + length), y)], fill=color, width=1)
        d.line([(x, y + sy * gap), (x, y + sy * (gap + length))], fill=color, width=1)


def print_bar(d, color):
    """Color control strip and registration target under the card."""
    colors = [(0, 174, 239), (236, 0, 140), (255, 242, 0), (46, 49, 146), (0, 166, 81), (237, 28, 36)]
    x, y = 305, 428
    for c in colors:
        d.rectangle([x, y, x + 15, y + 15], fill=c)
        x += 16
    cx, cy, r = 437, 436, 10
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=color, width=2)
    d.ellipse([cx - 5, cy - 5, cx + 5, cy + 5], fill=color)
    d.line([(cx - r - 4, cy), (cx + r + 4, cy)], fill=color, width=1)
    d.line([(cx, cy - r - 4), (cx, cy + r + 4)], fill=color, width=1)


def sketches(d, color):
    """Faint outlines of what PubWell makes: a flyer, a business card, a label sheet."""
    # Flyer: headline bar, picture box, text lines
    x, y, w, h = 40, 232, 92, 122
    d.rounded_rectangle([x, y, x + w, y + h], radius=3, outline=color, width=2)
    d.rectangle([x + 10, y + 10, x + w - 10, y + 22], fill=color)
    d.rectangle([x + 10, y + 30, x + w - 10, y + 70], outline=color, width=2)
    for i in range(4):
        d.line([(x + 10, y + 80 + i * 9), (x + w - 10 - (18 if i == 3 else 0), y + 80 + i * 9)], fill=color, width=2)
    # Business card
    x, y, w, h = 150, 290, 96, 56
    d.rounded_rectangle([x, y, x + w, y + h], radius=3, outline=color, width=2)
    d.ellipse([x + 10, y + 12, x + 28, y + 30], outline=color, width=2)
    for i, lw in enumerate([46, 34, 40]):
        d.line([(x + 38, y + 16 + i * 9), (x + 38 + lw, y + 16 + i * 9)], fill=color, width=2)
    # Label sheet
    x, y, w, h = 150, 232, 96, 48
    d.rounded_rectangle([x, y, x + w, y + h], radius=3, outline=color, width=2)
    for r in range(2):
        for c in range(3):
            lx, ly = x + 7 + c * 29, y + 7 + r * 18
            d.rounded_rectangle([lx, ly, lx + 24, ly + 14], radius=3, outline=color, width=1)


def splash(theme):
    t = THEMES[theme]
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle(CARD, fill=t["card"] + (255,))
    crop_marks(d, t["marks"])
    print_bar(d, t["marks"])

    # The pages break out above the card, like the pen in the Scribus splash.
    icon = render(136)
    img.alpha_composite(icon, (34, 34))

    mark = wordmark(50, t["ink"])
    img.alpha_composite(mark, (190, 112))

    sketches(d, t["line"])

    sub = ImageFont.truetype(FONT_SEMIBOLD, 16)
    d.text((193, 172), "Desktop publishing for everyone", font=sub, fill=t["sub"])
    small = ImageFont.truetype(FONT, 12)
    d.text((32, 384), "Based on Scribus  \u00b7  Free software (GPL)", font=small, fill=t["sub"])
    return img


def main():
    os.makedirs(OUT, exist_ok=True)
    for theme in THEMES:
        path = os.path.join(OUT, f"pubwell_splash_{theme}.png")
        splash(theme).save(path)
        print("wrote", path)


if __name__ == "__main__":
    main()
