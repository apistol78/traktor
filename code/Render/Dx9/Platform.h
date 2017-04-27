/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Platform_H
#define traktor_render_Platform_H

#if defined (_WIN32)
#	if defined (_XBOX)
#		include <xtl.h>
#		include <d3d9.h>
#		include <d3dx9.h>
#		include <xgraphics.h>
#	elif defined (T_USE_XDK)
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#		include <d3d9.h>
#		include <d3dx9.h>
#		include <xgraphics.h>
#	else
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#		include <d3dx9.h>
#		include <d3d9types.h>
#	endif
#		include <tchar.h>
#endif

#endif	// traktor_render_Platform_H
