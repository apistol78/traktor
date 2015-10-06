#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Resource/ResourceClassFactory.h"

namespace traktor
{
	namespace resource
	{

extern "C" void __module__Traktor_Resource()
{
	T_FORCE_LINK_REF(ResourceClassFactory);
}

	}
}

#endif
