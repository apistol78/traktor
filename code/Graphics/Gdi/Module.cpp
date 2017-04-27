/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Graphics/Gdi/GraphicsSystemGdi.h"

namespace traktor
{
	namespace graphics
	{

extern "C" void __module__Traktor_Graphics_Gdi()
{
	T_FORCE_LINK_REF(GraphicsSystemGdi);
}

	}
}

#endif
