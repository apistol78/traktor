@echo off

set EXECUTABLE=%1

set LIBRARIES=^
 "Traktor.Amalgam.App.lib"^
 "Traktor.Animation.lib"^
 "Traktor.Core.lib"^
 "Traktor.World.lib"^
 "Traktor.Render.lib"^
 "Traktor.Resource.lib"^
 "Traktor.Compress.lib"^
 "Traktor.Database.lib"^
 "Traktor.Database.Remote.lib"^
 "Traktor.Database.Remote.Client.lib"^
 "Traktor.Xml.lib"^
 "Traktor.Mesh.lib"^
 "Traktor.Physics.lib"^
 "Traktor.Physics.Bullet.lib"^
 "Traktor.Heightfield.lib"^
 "Traktor.Net.lib"^
 "Traktor.Drawing.lib"^
 "Traktor.Flash.lib"^
 "Traktor.Html.lib"^
 "Traktor.Sound.lib"^
 "Traktor.I18N.lib"^
 "Traktor.Input.lib"^
 "Traktor.Scene.lib"^
 "Traktor.Script.lib"^
 "Traktor.Script.Lua.lib"^
 "Traktor.Spray.lib"^
 "Traktor.Terrain.lib"^
 "Traktor.Theater.lib"^
 "Traktor.Weather.lib"^
 "Traktor.Video.lib"^
 "Traktor.Online.lib"^
 "Traktor.Amalgam.lib"

%EMSCRIPTEN%\emcc ^
	%LIBRARIES% ^
	-o %EXECUTABLE%.js
