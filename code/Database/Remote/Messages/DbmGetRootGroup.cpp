#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetRootGroup", 0, DbmGetRootGroup, IMessage)

void DbmGetRootGroup::serialize(ISerializer& s)
{
}

	}
}
