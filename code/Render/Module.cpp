#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/RenderClassFactory.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render()
{
	T_FORCE_LINK_REF(RenderClassFactory);
}

	}
}

#endif
