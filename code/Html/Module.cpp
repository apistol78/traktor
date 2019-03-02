#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Html/HtmlClassFactory.h"

namespace traktor
{
	namespace html
	{

extern "C" void __module__Traktor_Html()
{
	T_FORCE_LINK_REF(HtmlClassFactory);
}

	}
}

#endif
