@echo off

call %~dp0config.bat

set MAKE_DIALECT=nmake
set MAKE_TOOLCHAIN=clang
set MAKE_OS=windows
set MAKE_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\make-config-emscripten.inc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.bc
set MAKE_SHARED_LIBRARY_FILE=lib%%s.bc
set MAKE_EXECUTABLE_FILE=%%s.js
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\emscripten

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project-emscripten.sbm ^
	%TRAKTOR_HOME%\resources\build\ExternEmscripten.xms

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project-emscripten.sbm ^
	%TRAKTOR_HOME%\resources\build\TraktorEmscripten.xms
