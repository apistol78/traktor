#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Ps4/RenderSystemPs4.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Ps4()
{
	T_FORCE_LINK_REF(RenderSystemPs4);
}

	}
}

#endif
