@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

:: Copy PPU archives and binaries
mkdir bin\latest\ps3\ppu\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\debugstatic\*.lib bin\latest\ps3\ppu\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\debugstatic\*.elf bin\latest\ps3\ppu\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\debugstatic\*.self bin\latest\ps3\ppu\debugstatic

mkdir bin\latest\ps3\ppu\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\releasestatic\*.lib bin\latest\ps3\ppu\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\releasestatic\*.elf bin\latest\ps3\ppu\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3\releasestatic\*.self bin\latest\ps3\ppu\releasestatic

:: Copy SPU archives and binaries
mkdir bin\latest\ps3\spu\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3-spu\debugstatic\*.lib bin\latest\ps3\spu\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3-spu\debugstatic\*.o bin\latest\ps3\spu\debugstatic

mkdir bin\latest\ps3\spu\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3-spu\releasestatic\*.lib bin\latest\ps3\spu\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps3-spu\releasestatic\*.o bin\latest\ps3\spu\releasestatic

popd
