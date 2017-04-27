/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Dx11/RenderSystemDx11.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Dx11()
{
	T_FORCE_LINK_REF(RenderSystemDx11);
}

	}
}

#endif
