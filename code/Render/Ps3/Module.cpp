#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Ps3/RenderSystemPs3.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Ps3()
{
	T_FORCE_LINK_REF(RenderSystemPs3);
}

	}
}

#endif
