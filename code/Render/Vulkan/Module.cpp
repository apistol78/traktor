#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Vulkan/RenderSystemVk.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render_Vk()
{
	T_FORCE_LINK_REF(RenderSystemVk);
}

	}
}

#endif
