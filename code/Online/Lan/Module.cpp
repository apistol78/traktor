#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Online/Lan/LanGameConfiguration.h"
#	include "Online/Lan/LanSessionManager.h"

namespace traktor
{
	namespace online
	{

extern "C" void __module__Traktor_Online_Lan()
{
	T_FORCE_LINK_REF(LanGameConfiguration);
	T_FORCE_LINK_REF(LanSessionManager);
}

	}
}

#endif
