@echo off
rem DarkHorse: build Scribus x64 with MSVC 2022 (v143). Usage: build-scribus-x64.bat [Release|Debug]
set CFG=%1
if "%CFG%"=="" set CFG=Release
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 || exit /b 1
msbuild.exe C:\Developer\scribus\win32\msvc2022\Scribus.sln /t:Build /m /p:Configuration=%CFG% /p:Platform=x64 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0 /p:CL_MPCount=%NUMBER_OF_PROCESSORS% /v:minimal /nologo /fl /flp:logfile=C:\Developer\logs\scribus-%CFG%.log;verbosity=normal
