"""Build the PubWell icon set from concept B ("Shared Pages").

The shapes are traced from concept-b-chatgpt.png and rebuilt as clean geometry,
so every size is drawn sharp instead of scaled from a raster.

Outputs (next to this script, in out/):
  pubwell-icon.svg          master vector, transparent background
  pubwell-icon-<n>.png      n = 16 ... 1024
  pubwell.ico               Windows icon, 16-256 px
  preview.png               all sizes on light and dark backgrounds

Requires: Pillow, shapely.  Run: python make_icons.py
"""
import os
from PIL import Image, ImageDraw
from shapely.geometry import Polygon
from shapely import affinity

PLUM = (126, 63, 95)          # #7E3F5F, sampled from the concept
INK = (42, 45, 49)            # wordmark color in the concept
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")

# Traced in concept image pixels (the large icon spans about 122..416 x 180..468).
GAP = 21          # white gap between the pages
RADIUS = 14       # corner rounding


def page(x0, y_top_left, width, slope):
    """One sheet: slanted top edge, straight sides, curved (well) bottom."""
    pts = [(x0, y_top_left), (x0 + width, y_top_left + slope * width)]
    right_bottom = (x0 + width, 430)
    left_bottom = (x0, 400)
    ctrl = (x0 + 118, 520)
    pts.append(right_bottom)
    steps = 48
    for i in range(1, steps):          # quadratic bezier from right to left
        t = i / steps
        bx = (1 - t) ** 2 * right_bottom[0] + 2 * (1 - t) * t * ctrl[0] + t * t * left_bottom[0]
        by = (1 - t) ** 2 * right_bottom[1] + 2 * (1 - t) * t * ctrl[1] + t * t * left_bottom[1]
        pts.append((bx, by))
    pts.append(left_bottom)
    return Polygon(pts)


def rounded(poly, r):
    return poly.buffer(-r, join_style="round").buffer(r, join_style="round")


def shapes(gap=GAP, radius=RADIUS):
    front_raw = page(122, 240, 222, 0.19)
    back_raw = affinity.translate(front_raw, 72, -55)
    front = rounded(front_raw, radius)
    back = rounded(back_raw, radius).difference(front.buffer(gap, join_style="round"))
    return front, back


def bounds_transform(size, pad_frac=0.04):
    """Map traced coordinates onto a size x size square, centered."""
    front, back = shapes()
    minx, miny, maxx, maxy = front.union(back).bounds
    w, h = maxx - minx, maxy - miny
    scale = size * (1 - 2 * pad_frac) / max(w, h)
    ox = (size - w * scale) / 2 - minx * scale
    oy = (size - h * scale) / 2 - miny * scale
    return scale, ox, oy


def place(geom, scale, ox, oy):
    g = affinity.scale(geom, scale, scale, origin=(0, 0))
    return affinity.translate(g, ox, oy)


def polys(geom):
    return list(geom.geoms) if hasattr(geom, "geoms") else [geom]


def render(size, color=PLUM, ss=8):
    big = size * ss
    scale, ox, oy = bounds_transform(big)
    # At tiny sizes the traced gap would fall under a pixel; keep it at least ~1.3 px.
    gap = max(GAP, 1.3 * ss / scale)
    front, back = shapes(gap=gap)
    img = Image.new("L", (big, big), 0)
    d = ImageDraw.Draw(img)
    for geom in (back, front):
        for p in polys(place(geom, scale, ox, oy)):
            d.polygon(list(p.exterior.coords), fill=255)
            for hole in p.interiors:
                d.polygon(list(hole.coords), fill=0)
    mask = img.resize((size, size), Image.LANCZOS)
    out = Image.new("RGBA", (size, size), color + (0,))
    out.putalpha(mask)
    return out


def svg(size=1024):
    scale, ox, oy = bounds_transform(size)
    front, back = shapes()
    paths = []
    for geom in (back, front):
        for p in polys(place(geom, scale, ox, oy)):
            ring = lambda c: "M" + " L".join(f"{x:.1f},{y:.1f}" for x, y in c) + " Z"
            paths.append(ring(p.exterior.coords) + "".join(ring(h.coords) for h in p.interiors))
    body = "\n".join(f'  <path d="{d}"/>' for d in paths)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {size} {size}" '
            f'width="{size}" height="{size}">\n<g fill="#{PLUM[0]:02X}{PLUM[1]:02X}{PLUM[2]:02X}" '
            f'fill-rule="evenodd">\n{body}\n</g>\n</svg>\n')


def main():
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, "pubwell-icon.svg"), "w", encoding="utf-8") as f:
        f.write(svg())
    sizes = [16, 20, 24, 32, 40, 48, 64, 128, 256, 512, 1024]
    imgs = {n: render(n) for n in sizes}
    for n, im in imgs.items():
        im.save(os.path.join(OUT, f"pubwell-icon-{n}.png"))
    ico_sizes = [16, 20, 24, 32, 40, 48, 64, 128, 256]
    imgs[256].save(os.path.join(OUT, "pubwell.ico"), sizes=[(n, n) for n in ico_sizes],
                   append_images=[imgs[n] for n in ico_sizes if n != 256])

    # Preview: actual pixel sizes, plus 8x zoom of the small ones, light and dark.
    show = [256, 128, 64, 48, 32, 24, 16]
    W = sum(show) + 40 * len(show) + 40
    zoom = [16, 24, 32]
    H = 2 * (256 + 60) + 2 * (32 * 8 + 40)
    sheet = Image.new("RGB", (max(W, 3 * (32 * 8 + 40) + 40), H), (255, 255, 255))
    for row, bg in enumerate([(250, 248, 244), (32, 32, 32)]):
        y0 = row * (256 + 60)
        ImageDraw.Draw(sheet).rectangle([0, y0, sheet.width, y0 + 256 + 60], fill=bg)
        x = 40
        for n in show:
            sheet.paste(imgs[n], (x, y0 + 30 + (256 - n) // 2), imgs[n])
            x += n + 40
        y1 = 2 * (256 + 60) + row * (32 * 8 + 40)
        ImageDraw.Draw(sheet).rectangle([0, y1, sheet.width, y1 + 32 * 8 + 40], fill=bg)
        x = 40
        for n in zoom:
            z = imgs[n].resize((n * 8, n * 8), Image.NEAREST)
            sheet.paste(z, (x, y1 + 20), z)
            x += 32 * 8 + 40
    sheet.save(os.path.join(OUT, "preview.png"))
    print("wrote", OUT)


if __name__ == "__main__":
    main()
