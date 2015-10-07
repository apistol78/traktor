#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Database/Compact/CompactDatabase.h"

namespace traktor
{
	namespace db
	{

extern "C" void __module__Traktor_Database_Compact()
{
	T_FORCE_LINK_REF(CompactDatabase);
}

	}
}

#endif
