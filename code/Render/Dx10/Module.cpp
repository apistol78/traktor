#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Dx10/RenderSystemDx10.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Dx10()
{
	T_FORCE_LINK_REF(RenderSystemDx10);
}

	}
}

#endif
