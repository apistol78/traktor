#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Online/OnlineClassFactory.h"

namespace traktor
{
	namespace online
	{

extern "C" void __module__Traktor_Online()
{
	T_FORCE_LINK_REF(OnlineClassFactory);
}

	}
}

#endif
