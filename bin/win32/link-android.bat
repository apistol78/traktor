@echo off

set EXECUTABLE=%1.so

set CC=%ANDROID_NDK_ROOT%\toolchains\arm-linux-androideabi-4.6\prebuilt\windows\bin\arm-linux-androideabi-c++.exe

set LIBS_PATH=%TRAKTOR_HOME%\bin\latest\android\releasestatic
set LIBRARIES=^
 "%LIBS_PATH%\libTraktor.Animation.a"^
 "%LIBS_PATH%\libTraktor.Core.a"^
 "%LIBS_PATH%\libTraktor.World.a"^
 "%LIBS_PATH%\libTraktor.Render.a"^
 "%LIBS_PATH%\libTraktor.Resource.a"^
 "%LIBS_PATH%\libTraktor.Compress.a"^
 "%LIBS_PATH%\libTraktor.Database.a"^
 "%LIBS_PATH%\libTraktor.Database.Remote.a"^
 "%LIBS_PATH%\libTraktor.Database.Remote.Client.a"^
 "%LIBS_PATH%\libTraktor.Xml.a"^
 "%LIBS_PATH%\libTraktor.Mesh.a"^
 "%LIBS_PATH%\libTraktor.Physics.a"^
 "%LIBS_PATH%\libTraktor.Physics.Bullet.a"^
 "%LIBS_PATH%\libTraktor.Heightfield.a"^
 "%LIBS_PATH%\libTraktor.Net.a"^
 "%LIBS_PATH%\libTraktor.Drawing.a"^
 "%LIBS_PATH%\libTraktor.Flash.a"^
 "%LIBS_PATH%\libTraktor.Html.a"^
 "%LIBS_PATH%\libTraktor.Sound.a"^
 "%LIBS_PATH%\libTraktor.I18N.a"^
 "%LIBS_PATH%\libTraktor.Input.a"^
 "%LIBS_PATH%\libTraktor.Scene.a"^
 "%LIBS_PATH%\libTraktor.Script.a"^
 "%LIBS_PATH%\libTraktor.Script.Lua.a"^
 "%LIBS_PATH%\libTraktor.Spray.a"^
 "%LIBS_PATH%\libTraktor.Terrain.a"^
 "%LIBS_PATH%\libTraktor.Theater.a"^
 "%LIBS_PATH%\libTraktor.Weather.a"^
 "%LIBS_PATH%\libTraktor.Video.a"^
 "%LIBS_PATH%\libTraktor.Online.a"^
 "%LIBS_PATH%\libTraktor.Amalgam.a"

echo "Linking %EXECUTABLE%..."

%CC% ^
 -o %EXECUTABLE% ^
 -Wl,--no-undefined ^
 "-lstdc++" ^
 "-lsupc++" ^
 "-lgnustl_static" ^
 "-lgcc" ^
 --sysroot="%ANDROID_NDK_ROOT%\platforms\android-9\arch-arm" ^
 -L"%ANDROID_NDK_ROOT%\platforms\android-9\arch-arm\usr\lib" ^
 -L"%ANDROID_NDK_ROOT%\sources\cxx-stl\gnu-libstdc++\4.6\libs\armeabi" ^
 -L"%TRAKTOR_HOME%\bin\latest\android\releasestatic" ^
 -lc -lm -llog -lgcc -ldl ^
 -shared ^
 -Wl,-z,noexecstack ^
 -nostdlib ^
 %LIBRARIES%
