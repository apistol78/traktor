@echo off

set EMSDK=%TRAKTOR_HOME:\=/%/3rdp/emsdk-master
set EMSCRIPTEN=%EMSDK%/emscripten/1.37.36
set CLANG=%EMSDK%/clang/e1.37.36_64bit
set BINARYEN=%EMSDK%/clang/e1.37.36_64bit/binaryen
set JAVA_HOME=%EMSDK%/java/8.152_64bit
set PYTHON=%EMSDK%/python/2.7.13.1_64bit
set NODE=%EMSDK%/node/8.9.1_64bit
set EMSCRIPTEN_BUILD_TEMP=%TRAKTOR_HOME:\=/%/build/emscripten/temp

set PATH=%CLANG%;%NODE%/bin;%PYTHON%/python-2.7.13.amd64;%JAVA_HOME%/bin;%EMSDK%;%EMSCRIPTEN%;%PATH%

%TRAKTOR_HOME%/bin/win64/releasestatic/Traktor.Run.App "%TRAKTOR_HOME%/scripts/misc/emscripten.template" > %USERPROFILE%/.emscripten
