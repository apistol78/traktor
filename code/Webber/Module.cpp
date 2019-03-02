#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Webber/WebberClassFactory.h"

namespace traktor
{
	namespace wbr
	{

extern "C" void __module__Traktor_Webber()
{
	T_FORCE_LINK_REF(WebberClassFactory);
}

	}
}

#endif
