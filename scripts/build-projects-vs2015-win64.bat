@call %~dp0config.bat

@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x64\release
@set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x64

if "%AGGREGATE_OUTPUT_PATH%" == "" (
	@set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win64
)

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-win64.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-win64.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
