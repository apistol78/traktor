@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-android.xml ^
	%TRAKTOR_HOME%\resources\build\ExternAndroid.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-android.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorAndroid.xms

