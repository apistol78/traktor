@echo off

rem Add Emscripten SDK to path.
pushd "C:\Program Files (x86)\Emscripten"
call emsdk_add_path.bat
popd

set EXECUTABLE=%1
set LIBPATH=%TRAKTOR_HOME%\bin\latest\emscripten\releasestatic

set LIBRARIES=^
 %LIBPATH%\Traktor.Amalgam.App.bc^
 %LIBPATH%\Traktor.Animation.bc^
 %LIBPATH%\Traktor.Core.bc^
 %LIBPATH%\Traktor.World.bc^
 %LIBPATH%\Traktor.Render.bc^
 %LIBPATH%\Traktor.Render.OpenGL.ES2.bc^
 %LIBPATH%\Traktor.Resource.bc^
 %LIBPATH%\Traktor.Compress.bc^
 %LIBPATH%\Traktor.Database.bc^
 %LIBPATH%\Traktor.Database.Remote.bc^
 %LIBPATH%\Traktor.Database.Remote.Client.bc^
 %LIBPATH%\Traktor.Xml.bc^
 %LIBPATH%\Traktor.Mesh.bc^
 %LIBPATH%\Traktor.Physics.bc^
 %LIBPATH%\Traktor.Physics.Bullet.bc^
 %LIBPATH%\Traktor.Heightfield.bc^
 %LIBPATH%\Traktor.Net.bc^
 %LIBPATH%\Traktor.Drawing.bc^
 %LIBPATH%\Traktor.Flash.bc^
 %LIBPATH%\Traktor.Html.bc^
 %LIBPATH%\Traktor.Sound.bc^
 %LIBPATH%\Traktor.I18N.bc^
 %LIBPATH%\Traktor.Input.bc^
 %LIBPATH%\Traktor.Scene.bc^
 %LIBPATH%\Traktor.Script.bc^
 %LIBPATH%\Traktor.Script.Lua.bc^
 %LIBPATH%\Traktor.Spray.bc^
 %LIBPATH%\Traktor.Sql.bc^
 %LIBPATH%\Traktor.Sql.Sqlite3.bc^
 %LIBPATH%\Traktor.Terrain.bc^
 %LIBPATH%\Traktor.Theater.bc^
 %LIBPATH%\Traktor.Weather.bc^
 %LIBPATH%\Traktor.Video.bc^
 %LIBPATH%\Traktor.Online.bc^
 %LIBPATH%\Traktor.Online.Local.bc^
 %LIBPATH%\Traktor.Amalgam.bc^
 %LIBPATH%\Traktor.Ai.bc^
 %LIBPATH%\Extern.expat-2.1.0.bc^
 %LIBPATH%\Extern.bullet.bc^
 %LIBPATH%\Extern.detour.bc^
 %LIBPATH%\Extern.recast.bc^
 %LIBPATH%\Extern.libogg-1.3.0.bc^
 %LIBPATH%\Extern.lua-5.1.4.bc^
 %LIBPATH%\Extern.jpeg-6b.bc^
 %LIBPATH%\Extern.minilzo-2.05.bc^
 %LIBPATH%\Extern.sqlite-3.7.6.3.bc^
 %LIBPATH%\Extern.stb_vorbis.bc^
 %LIBPATH%\Extern.zlib-1.2.3.bc^
 %LIBPATH%\Extern.liblzf-3.6.bc^
 %LIBPATH%\Extern.libflac-1.2.1.bc^
 %LIBPATH%\Extern.lpng128.bc^
 %LIBPATH%\Extern.libtheora-1.1.1.bc^
 %LIBPATH%\Extern.libvorbis-1.3.3.bc

rem emcc -O2 -g4 --js-opts 0 --closure 0 -s PRECISE_I64_MATH=1 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s TOTAL_MEMORY=536870912 %LIBRARIES% -o %EXECUTABLE%.html --preload-file Application.config
emcc -O2 -g4 --js-opts 0 --closure 0 -s PRECISE_I64_MATH=1 -s ASSERTIONS=1 -s SAFE_HEAP=0 -s TOTAL_MEMORY=536870912 %LIBRARIES% -o %EXECUTABLE%.html --preload-file Application.config
