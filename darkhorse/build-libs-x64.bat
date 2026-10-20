@echo off
rem DarkHorse: build Scribus 1.7.x dependency kit, x64 only (Release then Debug).
rem Same msbuild calls as the kit's build-all-libs.bat, minus the menus and Win32; kit pins SDK 8.1, so force the installed Win10/11 SDK.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 || exit /b 1
pushd C:\Developer\scribus-1.7.x-libs-msvc
for %%C in (Release Debug) do (
  msbuild.exe scribus-libs-msvc2022.sln /t:Build /m /p:Configuration=%%C /p:Platform=x64 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0 /p:CL_MPCount=%NUMBER_OF_PROCESSORS% /v:minimal /nologo /fl /flp:logfile=C:\Developer\logs\libs-%%C.log;verbosity=normal || (popd & exit /b 1)
)
popd

