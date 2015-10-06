#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Graphics/Gdi/GraphicsSystemGdi.h"

namespace traktor
{
	namespace graphics
	{

extern "C" void __module__Traktor_Graphics_Gdi()
{
	T_FORCE_LINK_REF(GraphicsSystemGdi);
}

	}
}

#endif
