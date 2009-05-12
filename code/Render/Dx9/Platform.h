#ifndef traktor_render_Platform_H
#define traktor_render_Platform_H

#include "Core/Platform.h"

#if defined (_WIN32)
#	if defined (_XBOX)
#		include <xtl.h>
#		include <d3d9.h>
#		include <d3dx9.h>
#		include <xgraphics.h>
#	elif defined (T_USE_XDK)
#		include <d3d9.h>
#		include <d3dx9.h>
#		include <xgraphics.h>
#	else
#		include <d3dx9.h>
#	endif
#		include <tchar.h>
#endif

#endif	// traktor_render_Platform_H
