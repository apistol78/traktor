#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Xml/XmlClassFactory.h"

namespace traktor
{
	namespace xml
	{

extern "C" void __module__Traktor_Xml()
{
	T_FORCE_LINK_REF(XmlClassFactory);
}

	}
}

#endif
