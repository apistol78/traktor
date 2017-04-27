/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Dx9/Win32/RenderSystemWin32.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Win32()
{
	T_FORCE_LINK_REF(RenderSystemWin32);
}

	}
}

#endif
