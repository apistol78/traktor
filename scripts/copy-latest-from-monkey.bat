@echo off

call %~dp0../config.bat

set BUILD_SERVER=http://intranet.doctore:8111


:: Get each platform binaries from build machines.

echo Downloading Android...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_Android/.lastFinished/artifacts.zip Traktor_Android.zip

echo Downloading iOS...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_IOS/.lastFinished/artifacts.zip Traktor_IOS.zip

echo Downloading Linux (x86)...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_LinuxX86/.lastFinished/artifacts.zip Traktor_LinuxX86.zip

echo Downloading OSX...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_OSX/.lastFinished/artifacts.zip Traktor_OSX.zip

echo Downloading PNaCl...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_PNaCl/.lastFinished/artifacts.zip Traktor_PNaCl.zip

echo Downloading Win32...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_Win32/.lastFinished/artifacts.zip Traktor_Win32.zip

echo Downloading Win64...
%TRAKTOR_HOME%\bin\win32\HttpGet %BUILD_SERVER%/guestAuth/repository/downloadAll/Traktor_Win64/.lastFinished/artifacts.zip Traktor_Win64.zip


:: Unzip into binaries folder.

echo Decompressing...

%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\android" Traktor_Android.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\ios" Traktor_IOS.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\linux" Traktor_LinuxX86.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\osx" Traktor_OSX.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\pnacl" Traktor_PNaCl.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\win32" Traktor_Win32.zip
%TRAKTOR_HOME%\bin\win32\7za x -y -o"%TRAKTOR_HOME%\bin\latest2\win64" Traktor_Win64.zip


:: Cleanup

echo Cleaning...

del Traktor_Android.zip
del Traktor_IOS.zip
del Traktor_LinuxX86.zip
del Traktor_OSX.zip
del Traktor_PNaCl.zip
del Traktor_Win32.zip
del Traktor_Win64.zip


:: Done

echo Done
