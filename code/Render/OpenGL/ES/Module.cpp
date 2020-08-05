#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/OpenGL/ES/RenderSystemOpenGLES.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_OpenGL_ES2()
{
	T_FORCE_LINK_REF(RenderSystemOpenGLES);
}

	}
}

#endif
