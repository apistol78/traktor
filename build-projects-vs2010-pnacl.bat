@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2010-pnacl.xml %TRAKTOR_HOME%\TraktorPNaCl.xms
