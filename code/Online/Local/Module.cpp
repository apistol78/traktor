#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Online/Local/LocalGameConfiguration.h"
#	include "Online/Local/LocalSessionManager.h"

namespace traktor
{
	namespace online
	{

extern "C" void __module__Traktor_Online_Local()
{
	T_FORCE_LINK_REF(LocalGameConfiguration);
	T_FORCE_LINK_REF(LocalSessionManager);
}

	}
}

#endif
