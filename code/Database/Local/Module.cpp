#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Database/Local/LocalDatabase.h"

namespace traktor
{
	namespace db
	{

extern "C" void __module__Traktor_Database_Local()
{
	T_FORCE_LINK_REF(LocalDatabase);
}

	}
}

#endif
