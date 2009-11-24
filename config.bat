@echo off

set TRAKTOR_HOME=%~dp0

set PATH=%PATH%;%TRAKTOR_HOME%/build/win32/releaseshared
set PATH=%PATH%;%TRAKTOR_HOME%/build/win32/debugshared

set PATH=%PATH%;%TRAKTOR_HOME%/3rdp/OpenGL ES 2.0 Emulator v1.4/bin
set PATH=%PATH%;%SCE_PS3_ROOT%/host-win32/Cg/bin

