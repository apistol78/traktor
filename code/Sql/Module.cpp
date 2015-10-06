#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sql/SqlClassFactory.h"

namespace traktor
{
	namespace sql
	{

extern "C" void __module__Traktor_Sql()
{
	T_FORCE_LINK_REF(SqlClassFactory);
}

	}
}

#endif
