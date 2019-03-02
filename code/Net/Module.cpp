#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Net/NetClassFactory.h"

namespace traktor
{
	namespace net
	{

extern "C" void __module__Traktor_Net()
{
	T_FORCE_LINK_REF(NetClassFactory);
}

	}
}

#endif
