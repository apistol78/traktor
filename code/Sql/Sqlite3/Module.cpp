#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace sql
	{

extern "C" void __module__Traktor_Sql_Sqlite3()
{
	T_FORCE_LINK_REF(ConnectionSqlite3);
}

	}
}

#endif
