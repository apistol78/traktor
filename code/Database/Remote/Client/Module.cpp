#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Database/Remote/Client/RemoteDatabase.h"

namespace traktor
{
	namespace db
	{

extern "C" void __module__Traktor_Database_Remote_Client()
{
	T_FORCE_LINK_REF(RemoteDatabase);
}

	}
}

#endif
