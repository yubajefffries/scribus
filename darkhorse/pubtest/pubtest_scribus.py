# Runs inside Scribus (-py). Opens one .pub, writes a JSON report, a PNG per
# page and an .sla copy into the output folder, then exits the process.
# Usage: PubWell.exe --prefs <profile> -ns -py pubtest_scribus.py <file.pub> <outdir>
import json
import os
import sys
import time
import traceback

import scribus

TYPE_NAMES = {2: "image", 4: "text", 5: "line", 6: "polygon", 7: "polyline",
              8: "pathtext", 12: "table", 15: "group"}


def main():
    src, outdir = sys.argv[1], sys.argv[2]
    os.makedirs(outdir, exist_ok=True)
    report = {"file": os.path.basename(src), "ok": False, "pages": []}
    try:
        t0 = time.time()
        report["opened"] = bool(scribus.openDoc(src))
        report["open_seconds"] = round(time.time() - t0, 2)
        if not scribus.haveDoc():
            raise RuntimeError("openDoc returned without a document")
        scribus.setRedraw(False)
        for page in range(1, scribus.pageCount() + 1):
            scribus.gotoPage(page)
            w, h = scribus.getPageSize()
            items = scribus.getPageItems()
            kinds, text_chars, fonts, images = {}, 0, set(), 0
            for name, typ, _order in items:
                kind = TYPE_NAMES.get(typ, str(typ))
                kinds[kind] = kinds.get(kind, 0) + 1
                if typ == 4:
                    try:
                        text_chars += scribus.getTextLength(name)
                        fonts.add(scribus.getFont(name))
                    except Exception:
                        pass
                elif typ == 2:
                    images += 1
            png = os.path.join(outdir, "page-%02d.png" % page)
            exp = scribus.ImageExport()
            exp.type = "PNG"
            exp.dpi = 72
            exp.scale = 100
            exp.saveAs(png)
            report["pages"].append({"page": page, "size_doc_units": [round(w, 1), round(h, 1)],
                                    "items": len(items), "kinds": kinds,
                                    "text_chars": text_chars, "fonts": sorted(fonts),
                                    "images": images, "png": os.path.basename(png)})
        scribus.saveDocAs(os.path.join(outdir, "imported.sla"))
        report["ok"] = True
    except Exception:
        report["error"] = traceback.format_exc()
    with open(os.path.join(outdir, "report.json"), "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
    os._exit(0)


main()
