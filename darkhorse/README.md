# DarkHorse fork of Scribus

DarkHorse IT fork of Scribus, the base for a Microsoft Publisher replacement
(Publisher reached end of support in October 2026). Upstream is the Scribus SVN
(`svn://scribus.net`), mirrored at `scribusproject/scribus` (remote `upstream`).
Our work lives on `darkhorse/dev`; keep `master` identical to upstream.

License: GPLv2+, same as Scribus. The fork stays open source.

## Windows dev setup (x64, MSVC 2022)

Everything lives under `C:\Developer` (short paths, no spaces; MSVC breaks on long paths).

| Piece | Location |
|---|---|
| Source (this repo) | `C:\Developer\scribus` |
| Visual Studio 2022 Community, C++ desktop workload, Win11 SDK | default install |
| Qt 6.11.2 msvc2022_64 (qtbase, qtsvg, qttools, qttranslations, qt5compat, qtimageformats) | `C:\Developer\Qt\6.11.2\msvc2022_64` |
| Scribus 1.7.x libs kit 20260412 (SourceForge `scribus-libs/scribus-1.7.x`, signed by Jean Ghali `CED28009CAC64F21`) | `C:\Developer\scribus-1.7.x-libs-msvc` |
| Build output | `C:\Developer\Scribus-builds\Scribus-<Config>-x64-v143` |

Qt 6.11 cannot be installed with aqtinstall 3.3.0 (new per-arch repo layout). Download the
`.7z` packages from `download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_6112/qt6_6112_msvc2022_64/`,
verify each against its `.sha256`, extract, and move the contents under `6.11.2\msvc2022_64`
(the archives have no version folder).

`win32/msvc2022/Scribus-build-props.props` points at the paths above.

## Build

```
darkhorse\build-libs-x64.bat            # once per libs kit, about 20 min on 2 cores
darkhorse\patch-libs-x64.bat            # apply darkhorse\patches to the kit, rebuild libmspub
darkhorse\build-scribus-x64.bat Release # about 20 min clean
darkhorse\deploy-runtime-x64.bat Release
```

## Patches to the libs kit

`darkhorse\patches\libmspub-0.1.4-*.patch` change libmspub inside the kit (it is linked
statically into the pub importer). `patch-libs-x64.bat` applies them with `git apply`
(skips ones already applied) and rebuilds libmspub; then rebuild Scribus.

- `picture-crop`: read Publisher's picture crop (OfficeArt 0x0100-0x0103) and pass it as
  `librevenge:crop-top/bottom/left/right` fractions. Upstream candidate for libmspub.

## .pub import test

`darkhorse\pubtest\run-pubtest.ps1` imports every `.pub` in `C:\Developer\pub-samples`
(one Scribus process each, throwaway profile) and writes `report.json`, page PNGs and an
`.sla` per file under `C:\Developer\pubtest-out`. Sample files are client documents and
stay out of the repo.

Gotchas:
- The libs kit pins Windows SDK 8.1 (and older Win10 SDKs); both build scripts pass
  `WindowsTargetPlatformVersion=10.0` to use the installed SDK.
- The kit's `copy-dlls-to-build-dir.bat` uses relative paths; run it from the kit folder.
- Without `qt.conf` (`Plugins=qtplugins`) Scribus fails with "no Qt platform plugin could be initialized".
