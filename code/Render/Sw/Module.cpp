#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Sw/RenderSystemSw.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Sw()
{
	T_FORCE_LINK_REF(RenderSystemSw);
}

	}
}

#endif
