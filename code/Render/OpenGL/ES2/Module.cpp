/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_OpenGL_ES2()
{
	T_FORCE_LINK_REF(RenderSystemOpenGLES2);
}

	}
}

#endif
