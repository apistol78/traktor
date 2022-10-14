#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/Vulkan/RenderSystemVk.h"

namespace traktor::render
{

extern "C" void __module__Traktor_Render_Vulkan()
{
	T_FORCE_LINK_REF(RenderSystemVk);
}

}

#endif
