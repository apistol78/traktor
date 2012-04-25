@echo off

set PACKAGE=%1
set EXECUTABLE=%2

rem %SCE_PS3_ROOT%

set LIBRARIES_PPU=^
 "ppu\Traktor.Amalgam.App.lib"^
 "ppu\Traktor.Animation.lib"^
 "ppu\Traktor.Core.lib"^
 "ppu\Traktor.World.lib"^
 "ppu\Traktor.Render.lib"^
 "ppu\Traktor.Render.Ps3.lib"^
 "ppu\Traktor.Resource.lib"^
 "ppu\Traktor.Compress.lib"^
 "ppu\Traktor.Database.lib"^
 "ppu\Traktor.Database.Remote.lib"^
 "ppu\Traktor.Database.Remote.Client.lib"^
 "ppu\Traktor.Xml.lib"^
 "ppu\Traktor.Mesh.lib"^
 "ppu\Traktor.Physics.lib"^
 "ppu\Traktor.Physics.Bullet.lib"^
 "ppu\Traktor.Heightfield.lib"^
 "ppu\Traktor.Net.lib"^
 "ppu\Traktor.Drawing.lib"^
 "ppu\Traktor.Flash.lib"^
 "ppu\Traktor.Html.lib"^
 "ppu\Traktor.Sound.lib"^
 "ppu\Traktor.Sound.Ps3.lib"^
 "ppu\Traktor.I18N.lib"^
 "ppu\Traktor.Input.lib"^
 "ppu\Traktor.Input.Ps3.lib"^
 "ppu\Traktor.Scene.lib"^
 "ppu\Traktor.Script.lib"^
 "ppu\Traktor.Script.Lua.lib"^
 "ppu\Traktor.Spray.lib"^
 "ppu\Traktor.Terrain.lib"^
 "ppu\Traktor.Theater.lib"^
 "ppu\Traktor.Weather.lib"^
 "ppu\Traktor.Video.lib"^
 "ppu\Traktor.Online.lib"^
 "ppu\Traktor.Amalgam.lib"^
 "ppu\Traktor.Parade.lib"

set LIBRARIES_EXTERN_PPU=^
 "ppu\Extern.bullet-2.80.lib"^
 "ppu\Extern.Expat.lib"^
 "ppu\Extern.jpeg-6b.lib"^
 "ppu\Extern.libflac-1.2.1.lib"^
 "ppu\Extern.liblzf-3.6.lib"^
 "ppu\Extern.libogg-1.2.0.lib"^
 "ppu\Extern.libtheora-1.1.1.lib"^
 "ppu\Extern.libvorbis-1.3.1.lib"^
 "ppu\Extern.lpng128.lib"^
 "ppu\Extern.lua-5.1.4.lib"^
 "ppu\Extern.minilzo-2.05.lib"^
 "ppu\Extern.mpg123-1.13.2.lib"^
 "ppu\Extern.zlib-1.2.3.lib"

set LIBRARIES_SYSTEM_PPU=^
 "libfs_stub.a"^
 "libio_stub.a"^
 "libm.a"^
 "libpthread.a"^
 "libsysmodule_stub.a"^
 "libsysutil_game_stub.a"^
 "libsysutil_stub.a"^
 "libnet_stub.a"^
 "libnetctl_stub.a"^
 "libspurs_stub.a"^
 "libspurs_jq_stub.a"^
 "libgcm_cmd.a"^
 "libgcm_pm.a"^
 "libgcm_sys_stub.a"^
 "libsysutil_avconf_ext_stub.a"^
 "libsysutil_np_stub.a"^
 "libaudio_stub.a"

set LIBRARIES_SPU=^
 "spu\Traktor.Sound.JobAddMulConst.lib"^
 "spu\Traktor.Sound.JobMulConst.lib"^
 "spu\Traktor.Sound.JobStretch.lib"^
 "spu\Traktor.Spray.JobDragModifier.lib"^
 "spu\Traktor.Spray.JobGravityModifier.lib"^
 "spu\Traktor.Spray.JobIntegrateModifier.lib"^
 "spu\Traktor.Spray.JobPlaneCollisionModifier.lib"^
 "spu\Traktor.Spray.JobPointRenderer.lib"^
 "spu\Traktor.Spray.JobSizeModifier.lib"^
 "spu\Traktor.Spray.JobVortexModifier.lib"

set LD=ps3ppuld
set SIGN=make_fself

%LD% ^
	--whole-archive %LIBRARIES_PPU% ^
	--no-whole-archive %LIBRARIES_EXTERN_PPU% %LIBRARIES_SYSTEM_PPU% %LIBRARIES_SPU% ^
	-o %EXECUTABLE%.elf

%SIGN% %EXECUTABLE%.elf %EXECUTABLE%.self
