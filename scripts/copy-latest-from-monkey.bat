@echo off

call %~dp0../config.bat

set BUILD_SERVER=http://intranet.doctore:8111


:: Get each platform binaries from build machines.

echo Downloading Android...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Android/.lastFinished/traktor_android.zip traktor_android.zip

echo Downloading iOS...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_IOS/.lastFinished/traktor_ios.zip traktor_ios.zip

echo Downloading Linux (x86)...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_LinuxX86/.lastFinished/traktor_linux_x86.zip traktor_linux_x86.zip

echo Downloading OSX...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_OSX/.lastFinished/traktor_osx.zip traktor_osx.zip

echo Downloading PNaCl...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_PNaCl/.lastFinished/traktor_pnacl.zip traktor_pnacl.zip

echo Downloading Emscripten...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_PNaCl/.lastFinished/traktor_emscripten.zip traktor_emscripten.zip

echo Downloading Win32...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_win32.zip traktor_win32.zip

echo Downloading Win64...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_win64.zip traktor_win64.zip

echo Downloading Mobile6...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/download/Traktor_Win32/.lastFinished/traktor_mobile6.zip traktor_mobile6.zip


:: Ensure old binaries folder is cleaned.

echo Preparing...

rmdir /s /q "%TRAKTOR_HOME%\bin\latest"


:: Unzip into binaries folder.

echo Decompressing...

%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\android" traktor_android.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\ios" traktor_ios.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\linux" traktor_linux_x86.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\osx" traktor_osx.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\pnacl" traktor_pnacl.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\emscripten" traktor_emscripten.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\win32" traktor_win32.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\win64" traktor_win64.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest\mobile6" traktor_mobile6.zip


:: Cleanup

echo Cleaning...

del *.zip


:: Done

echo Done
