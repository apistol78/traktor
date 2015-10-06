#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/OpenGL/Std/RenderSystemOpenGL.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_OpenGL()
{
	T_FORCE_LINK_REF(RenderSystemOpenGL);
}

	}
}

#endif
