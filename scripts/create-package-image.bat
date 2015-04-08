@echo off

call %~dp0../config.bat

mkdir "package-image"

::
:: BINARIES
::

:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\ios\releasestatic\*.*" "package-image\bin\latest\ios\releasestatic\"

:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\linux\releaseshared\*.*" "package-image\bin\latest\linux\releaseshared\"

:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\osx\releaseshared\*.*" "package-image\bin\latest\osx\releaseshared\"

:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\ps3\ppu\releasestatic\*.*" "package-image\bin\latest\ps3\ppu\releasestatic\"
:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\ps3\spu\releasestatic\*.*" "package-image\bin\latest\ps3\spu\releasestatic\"

:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\win32\releaseshared\*.dll" "package-image\bin\latest\win32\releaseshared\"
:: xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\win32\releaseshared\*.exe" "package-image\bin\latest\win32\releaseshared\"

xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\win64\releaseshared\*.dll" "package-image\bin\latest\win64\releaseshared\"
xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\latest\win64\releaseshared\*.exe" "package-image\bin\latest\win64\releaseshared\"

::
:: MISC
::

xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\deploy\*.*" "package-image\bin\deploy\"
xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\template\*.*" "package-image\bin\template\"
xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\bin\win32\*.*" "package-image\bin\win32\"

::
:: SYSTEM DATA
::

xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\data\Assets\*.*" "package-image\data\Assets\"
xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\data\Source\*.*" "package-image\data\Source\"

::
:: RES
::

xcopy /C /I /R /Y "%TRAKTOR_HOME%\res\English.xml" "package-image\res\"
xcopy /C /I /R /Y "%TRAKTOR_HOME%\res\Help.xml" "package-image\res\"
xcopy /C /I /R /Y /E "%TRAKTOR_HOME%\res\templates\*.*" "package-image\res\templates\"

::
:: (root)
::

copy /Y "%TRAKTOR_HOME%\config.bat" "package-image\"
copy /Y "%TRAKTOR_HOME%\run-editor-win64.bat" "package-image\"
copy /Y "%TRAKTOR_HOME%\Traktor.Editor.config" "package-image\"
copy /Y "%TRAKTOR_HOME%\Traktor.Editor.win32.config" "package-image\"