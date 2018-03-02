@echo off

call %~dp0../config.bat

set BUILD_SERVER=http://intranet.doctore:8111
set PLATFORM="%1"

:: Get each platform binaries from build machines.

if /i %PLATFORM% == "" ( goto :download_android )
if /i %PLATFORM% == "android" ( goto :download_android )
goto :skip_android
:download_android
echo Downloading Android...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Android/.lastFinished/traktor_android.zip traktor_android.zip
:skip_android

if /i %PLATFORM% == "" ( goto :download_ios )
if /i %PLATFORM% == "ios" ( goto :download_ios )
goto :skip_ios
:download_ios
echo Downloading iOS...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_IOS/.lastFinished/traktor_ios.zip traktor_ios.zip
:skip_ios

if /i %PLATFORM% == "" ( goto :download_linux_x86 )
if /i %PLATFORM% == "linux32" ( goto :download_linux_x86 )
goto :skip_linux_x86
:download_linux_x86
echo Downloading Linux (x86)...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_LinuxX86/.lastFinished/traktor_linux_x86.zip traktor_linux_x86.zip
:skip_linux_x86

if /i %PLATFORM% == "" ( goto :download_osx )
if /i %PLATFORM% == "osx" ( goto :download_osx )
goto :skip_osx
:download_osx
echo Downloading OSX...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_OSX/.lastFinished/traktor_osx.zip traktor_osx.zip
:skip_osx

if /i %PLATFORM% == "" ( goto :download_pnacl )
if /i %PLATFORM% == "pnacl" ( goto :download_pnacl )
goto :skip_pnacl
:download_pnacl
echo Downloading PNaCl...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_PNaCl/.lastFinished/traktor_pnacl.zip traktor_pnacl.zip
:skip_pnacl

if /i %PLATFORM% == "" ( goto :download_emscripten )
if /i %PLATFORM% == "emscripten" ( goto :download_emscripten )
goto :skip_emscripten
:download_emscripten
echo Downloading Emscripten...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_PNaCl/.lastFinished/traktor_emscripten.zip traktor_emscripten.zip
:skip_emscripten

if /i %PLATFORM% == "" ( goto :download_win32 )
if /i %PLATFORM% == "win32" ( goto :download_win32 )
goto :skip_win32
:download_win32
echo Downloading Win32...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_win32.zip traktor_win32.zip
:skip_win32

if /i %PLATFORM% == "" ( goto :download_win64 )
if /i %PLATFORM% == "win64" ( goto :download_win64 )
goto :skip_win64
:download_win64
echo Downloading Win64...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_win64.zip traktor_win64.zip
:skip_win64

if /i %PLATFORM% == "" ( goto :download_mobile6 )
if /i %PLATFORM% == "mobile6" ( goto :download_mobile6 )
goto :skip_mobile6
:download_mobile6
echo Downloading Mobile6...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_mobile6.zip traktor_mobile6.zip
:skip_mobile6

if /i %PLATFORM% == "" ( goto :download_ps3 )
if /i %PLATFORM% == "ps3" ( goto :download_ps3 )
goto :skip_ps3
:download_ps3
echo Downloading PS3...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_PNaCl/.lastFinished/traktor_ps3.zip traktor_ps3.zip
:skip_ps3

if /i %PLATFORM% == "" ( goto :download_doc )
if /i %PLATFORM% == "doc" ( goto :download_doc )
goto :skip_doc
:download_doc
echo Downloading Documentation...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_doc.zip traktor_doc.zip
:skip_doc

:: Unzip into binaries folder.

echo Decompressing...

if not exist traktor_android.zip ( goto :no_android )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\android"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\android" traktor_android.zip
:no_android

if not exist traktor_ios.zip ( goto :no_ios )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\ios"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\ios" traktor_ios.zip
:no_ios

if not exist traktor_linux_x86.zip ( goto :no_linux_x86 )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\linux"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\linux" traktor_linux_x86.zip
:no_linux_x86

if not exist traktor_osx.zip ( goto :no_osx )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\osx"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\osx" traktor_osx.zip
:no_osx

if not exist traktor_pnacl.zip ( goto :no_pnacl )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\pnacl"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\pnacl" traktor_pnacl.zip
:no_pnacl

if not exist traktor_emscripten.zip ( goto :no_emscripten )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\emscripten"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\emscripten" traktor_emscripten.zip
:no_emscripten

if not exist traktor_win32.zip ( goto :no_win32 )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\win32"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\win32" traktor_win32.zip
:no_win32

if not exist traktor_win64.zip ( goto :no_win64 )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\win64"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\win64" traktor_win64.zip
:no_win64

if not exist traktor_mobile6.zip ( goto :no_mobile6 )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\mobile6"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\mobile6" traktor_mobile6.zip
:no_mobile6

if not exist traktor_ps3.zip ( goto :no_ps3 )
rmdir /s /q "%TRAKTOR_HOME%\bin\latest\ps3"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\ps3" traktor_ps3.zip
:no_ps3

if not exist traktor_doc.zip ( goto :no_doc )
rmdir /s /q "%TRAKTOR_HOME%\doc\latest"
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\doc\latest" traktor_doc.zip
:no_doc

:: Cleanup

echo Cleaning...
del *.zip

:: Done

echo Done
