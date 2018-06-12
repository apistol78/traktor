@echo off

call %~dp0config.bat

set FBUILD_TOOLCHAIN=clang
set FBUILD_MANIFEST=%TRAKTOR_HOME%\resources\build\windows\manifest-win64.xml
set FBUILD_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\fbuild-config-emscripten.bff

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\ExternEmscripten.xms

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\TraktorEmscripten.xms
