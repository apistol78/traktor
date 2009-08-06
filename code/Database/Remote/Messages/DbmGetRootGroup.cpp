#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmGetRootGroup", DbmGetRootGroup, IMessage)

bool DbmGetRootGroup::serialize(Serializer& s)
{
	return true;
}

	}
}
