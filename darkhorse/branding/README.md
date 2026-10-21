# PubWell branding

Concept B ("Shared Pages"), picked by Jeff on 2026-10-20. Two sheets of paper, the front one with a curved
"well" bottom. Plum `#7E3F5F`, wordmark ink `#2A2D31`.

- `concept-b-chatgpt.png`: the original concept sheet (ChatGPT image). Reference only.
- `make_icons.py`: rebuilds the symbol as clean geometry traced from the concept and writes `out/`
  (SVG master, PNGs 16 to 1024 px, `pubwell.ico`, `preview.png`). Needs Pillow and shapely.
  At small sizes the gap between the pages is kept to at least about 1.3 px so it stays visible.

Where the icon is used (copy the files again after re-running the script):

| File | Used for |
|---|---|
| `win32/msvc2022/scribus-main/pubwell.ico` | Program file icon (`Scribus.rc`, `SCRIBUSICO`) |
| `resources/iconsets/1_7_0/pubwell-icon.svg` | `app-icon` in `1_7_0.xml`: window icons and the first-run wizard logo |

Still Scribus-branded: the document icon (`scribusdoc.ico`) and the wordmark in the app's text.
