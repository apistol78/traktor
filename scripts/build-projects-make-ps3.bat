@echo off

call %~dp0config.bat

:: PPU

:: Mandatory parameters.
set MAKE_OS=windows
set MAKE_DIALECT=nmake
set MAKE_FORMAT=%TRAKTOR_HOME%\resources\build\configurations\make-format.sbm
set MAKE_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\make-config-ps3-ppu.inc

:: Used by default format.
set MAKE_TOOLCHAIN=clang
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=%%s.lib
set MAKE_SHARED_LIBRARY_FILE=%%s.lib
set MAKE_EXECUTABLE_FILE=%%s.elf

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\ExternPs3.xms

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\TraktorPs3.xms

:: SPU

:: Mandatory parameters.
set MAKE_OS=windows
set MAKE_DIALECT=nmake
set MAKE_FORMAT=%TRAKTOR_HOME%\resources\build\configurations\make-format-ps3-spu.sbm
set MAKE_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\make-config-ps3-spu.inc

:: Used by default format.
set MAKE_TOOLCHAIN=clang
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.a
set MAKE_SHARED_LIBRARY_FILE=lib%%s.a
set MAKE_EXECUTABLE_FILE=%%s.elf

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\TraktorPs3-Spu.xms
