@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

set MAKE_DIALECT=nmake
set MAKE_TOOLCHAIN=clang
set MAKE_OS=windows
set MAKE_INCLUDE=$(TRAKTOR_HOME)\bin\make-config-pnacl-nmake.inc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.a
set MAKE_SHARED_LIBRARY_FILE=lib%%s.a
set MAKE_EXECUTABLE_FILE=%%s.js
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\pnacl

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=$(TRAKTOR_HOME)\bin\make-solution.sbm ^
	-make-project-template=$(TRAKTOR_HOME)\bin\make-project-emscripten.sbm ^
	%TRAKTOR_HOME%\TraktorPNaCl.xms
