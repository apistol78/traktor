@echo off

set EXECUTABLE=%1
set LIBPATH=%TRAKTOR_HOME%\bin\latest\pnacl\releasestatic
set CLANG=%NACL_SDK_ROOT%\toolchain\win_pnacl\bin\pnacl-clang++.bat 
set FINALIZE=%NACL_SDK_ROOT%\toolchain\win_pnacl\bin\pnacl-finalize.bat
set DEFAULT_LIBS=-lpthread -lppapi -lppapi_cpp -lnacl_io -lppapi_gles2

set TRAKTOR_LIBRARIES=^
 %LIBPATH%\Traktor.Amalgam.App.a^
 %LIBPATH%\Traktor.Amalgam.a^
 %LIBPATH%\Traktor.Animation.a^
 %LIBPATH%\Traktor.Scene.a^
 %LIBPATH%\Traktor.World.a^
 %LIBPATH%\Traktor.Mesh.a^
 %LIBPATH%\Traktor.Terrain.a^
 %LIBPATH%\Traktor.Render.OpenGL.ES2.a^
 %LIBPATH%\Traktor.Render.a^
 %LIBPATH%\Traktor.Resource.a^
 %LIBPATH%\Traktor.Database.Remote.Client.a^
 %LIBPATH%\Traktor.Database.Remote.a^
 %LIBPATH%\Traktor.Database.a^
 %LIBPATH%\Traktor.Xml.a^
 %LIBPATH%\Traktor.Physics.Bullet.a^
 %LIBPATH%\Traktor.Physics.a^
 %LIBPATH%\Traktor.Heightfield.a^
 %LIBPATH%\Traktor.Flash.a^
 %LIBPATH%\Traktor.Net.a^
 %LIBPATH%\Traktor.Compress.a^
 %LIBPATH%\Traktor.Drawing.a^
 %LIBPATH%\Traktor.Html.a^
 %LIBPATH%\Traktor.Sound.a^
 %LIBPATH%\Traktor.I18N.a^
 %LIBPATH%\Traktor.Input.a^
 %LIBPATH%\Traktor.Script.Lua.a^
 %LIBPATH%\Traktor.Script.a^
 %LIBPATH%\Traktor.Spray.a^
 %LIBPATH%\Traktor.Sql.Sqlite3.a^
 %LIBPATH%\Traktor.Sql.a^
 %LIBPATH%\Traktor.Theater.a^
 %LIBPATH%\Traktor.Weather.a^
 %LIBPATH%\Traktor.Video.a^
 %LIBPATH%\Traktor.Online.Local.a^
 %LIBPATH%\Traktor.Online.a^
 %LIBPATH%\Traktor.Ai.a^
 %LIBPATH%\Traktor.Core.a
 
set EXTERN_LIBRARIES=^
 %LIBPATH%\Extern.expat-2.1.0.a^
 %LIBPATH%\Extern.bullet.a^
 %LIBPATH%\Extern.detour.a^
 %LIBPATH%\Extern.recast.a^
 %LIBPATH%\Extern.libogg-1.3.0.a^
 %LIBPATH%\Extern.lua-5.1.4.a^
 %LIBPATH%\Extern.jpeg-6b.a^
 %LIBPATH%\Extern.minilzo-2.05.a^
 %LIBPATH%\Extern.mpg123-1.13.2.a^
 %LIBPATH%\Extern.sqlite-3.7.6.3.a^
 %LIBPATH%\Extern.stb_vorbis.a^
 %LIBPATH%\Extern.lpng128.a^
 %LIBPATH%\Extern.zlib-1.2.3.a^
 %LIBPATH%\Extern.liblzf-3.6.a^
 %LIBPATH%\Extern.libflac-1.2.1.a^
 %LIBPATH%\Extern.libtheora-1.1.1.a^
 %LIBPATH%\Extern.libvorbis-1.3.3.a

rem Link product.
echo %CLANG% -L%NACL_SDK_ROOT%\lib\pnacl\Release %DEFAULT_LIBS% -Xlinker --whole-archive %TRAKTOR_LIBRARIES% -Xlinker --no-whole-archive %EXTERN_LIBRARIES% %DEFAULT_LIBS% -o %EXECUTABLE%.unfinal.pexe > build.bat
call build.bat

rem Finalize product.
del %EXECUTABLE%.pexe
copy %EXECUTABLE%.unfinal.pexe %EXECUTABLE%.pexe
%FINALIZE% %EXECUTABLE%.pexe
