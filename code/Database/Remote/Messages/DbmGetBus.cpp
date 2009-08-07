#include "Database/Remote/Messages/DbmGetBus.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmGetBus", DbmGetBus, IMessage)

bool DbmGetBus::serialize(Serializer& s)
{
	return true;
}

	}
}
