@echo off

call %~dp0config.bat

set LEMON_SDK=%TRAKTOR_HOME%\3rdp\lemon

:: Generate parser C file.
%TRAKTOR_HOME%/build/win64/ReleaseStatic/Extern.lemon -T%LEMON_SDK%/lempar.c -l -q %TRAKTOR_HOME%/code/Script/Editor/LuaGrammar.y

:: Replace output with CPP file.
del %TRAKTOR_HOME%\code\Script\Editor\LuaGrammar.cpp
rename %TRAKTOR_HOME%\code\Script\Editor\LuaGrammar.c LuaGrammar.cpp