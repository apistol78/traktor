#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Database/DatabaseClassFactory.h"

namespace traktor
{
	namespace db
	{

extern "C" void __module__Traktor_Database()
{
	T_FORCE_LINK_REF(DatabaseClassFactory);
}

	}
}

#endif
