#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Shape/Runtime/RuntimePlugin.h"

namespace traktor
{
	namespace shape
	{

extern "C" void __module__Traktor_Shape_Runtime()
{
	T_FORCE_LINK_REF(RuntimePlugin);
}

	}
}

#endif
