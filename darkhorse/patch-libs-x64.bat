@echo off
rem DarkHorse: apply our libmspub patches to the extracted Scribus libs kit, then rebuild libmspub.
rem Safe to rerun: already-applied patches are skipped.
set KIT=C:\Developer\scribus-1.7.x-libs-msvc
for %%P in ("%~dp0patches\libmspub-0.1.4-*.patch") do (
  git -C "%KIT%\libmspub-0.1.4\source" apply --reverse --check --ignore-whitespace "%%~fP" 2>nul && (echo already applied: %%~nxP) || (
    git -C "%KIT%\libmspub-0.1.4\source" apply --ignore-whitespace "%%~fP" || exit /b 1
    echo applied: %%~nxP
  )
)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul || exit /b 1
for %%C in (Release Debug) do (
  msbuild.exe "%KIT%\libmspub-0.1.4\libmspub.vcxproj" /t:Build /m /p:Configuration=%%C /p:Platform=x64 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0 /v:minimal /nologo || exit /b 1
)
