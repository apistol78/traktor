@echo off

set EMSDK=%TRAKTOR_HOME:\=/%/3rdp/emsdk-master-200827

:: Use SDK provided script to setup environment.
call %EMSDK%/emsdk_env.bat
