@echo off
rem DarkHorse: make a built Scribus runnable. Usage: deploy-runtime-x64.bat [Release|Debug]
rem Copies the libs kit DLLs, deploys Qt DLLs + plugins into qtplugins\, and writes qt.conf.
rem qt.conf is required: Scribus sets the qtplugins path too late for the Qt platform plugin.
set CFG=%1
if "%CFG%"=="" set CFG=Release
set BUILDS=C:\Developer\Scribus-builds
set OUT=%BUILDS%\Scribus-%CFG%-x64-v143
if not exist "%OUT%\PubWell.exe" (echo %OUT%\PubWell.exe not found, build first & exit /b 1)
pushd C:\Developer\scribus-1.7.x-libs-msvc
call .\copy-dlls-to-build-dir.bat %BUILDS% >nul
@echo off
popd
set WDQ_MODE=--release
if /i "%CFG%"=="Debug" set WDQ_MODE=--debug
C:\Developer\Qt\6.11.2\msvc2022_64\bin\windeployqt.exe %WDQ_MODE% --no-compiler-runtime --plugindir "%OUT%\qtplugins" "%OUT%\PubWell.exe" >nul || exit /b 1
(echo [Paths]& echo Plugins=qtplugins) > "%OUT%\qt.conf"
echo Deployed runtime to %OUT%
