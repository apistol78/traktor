#include "Database/Remote/Messages/DbmGetBus.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetBus", 0, DbmGetBus, IMessage)

bool DbmGetBus::serialize(ISerializer& s)
{
	return true;
}

	}
}
