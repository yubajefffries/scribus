# PubWell branding

Concept B ("Shared Pages"), picked by Jeff on 2026-10-20. Two sheets of paper, the front one with a curved
"well" bottom. Plum `#7E3F5F`, wordmark ink `#2A2D31`.

- `concept-b-chatgpt.png`: the original concept sheet (ChatGPT image). Reference only.
- `make_icons.py`: rebuilds the symbol as clean geometry traced from the concept and writes `out/`
  (SVG master, PNGs 16 to 1024 px, `pubwell.ico`, `preview.png`). Needs Pillow and shapely.
  At small sizes the gap between the pages is kept to at least about 1.3 px so it stays visible.

- `make_splash.py`: the startup splash (light and dark, 490 x 454) with the icon, the wordmark cut from
  the concept image, crop marks and a print control strip. The bottom right of the card stays clear for
  the version number and loading messages that `ui/splash.cpp` draws.

Where the artwork is used (copy the files again after re-running the script):

| File | Used for |
|---|---|
| `win32/msvc2022/scribus-main/pubwell.ico` | Program file icon (`Scribus.rc`, `SCRIBUSICO`) |
| `resources/iconsets/1_7_0/pubwell-icon.svg` | `app-icon` in `1_7_0.xml`: window icons and the first-run wizard logo |
| `resources/iconsets/1_7_0/images/pubwell_splash_{light,dark}.png` | `splashscreen` in `1_7_0.xml`, also the About box picture |

## Name in the app

`scribus/branding.{h,cpp}`: `Branding::productName()` and a translator installed in front of the regular
ones that swaps "Scribus" for "PubWell" in every translated string, so no Scribus source string or
translation file changes (keeps upstream merges clean). It leaves "Scribus" where it really means Scribus:
version and file format names ("Scribus 1.6.0", "Scribus 1.7.0 Document"), the Scribus website, manual,
wiki and team, and "version of Scribus" / "Old Scribus Settings". Strings that must say Scribus no matter
what pass it as a `%1` argument (see the About box credit line).

- Program file is `PubWell.exe` (`TargetName` in `Scribus.vcxproj`).
- `QCoreApplication::setApplicationName("PubWell")` in `main_win32.cpp` / `main_nix.cpp`: settings live in
  `%APPDATA%\PubWell`, apart from a real Scribus install.
- About box: "PubWell is based on Scribus ..." credit line.

Still Scribus: the document icon (`scribusdoc.ico`), the version number (Scribus's), PDF "Creator", and the
update checker (it checks scribus.net; to disable or repoint).
