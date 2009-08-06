#include "Database/Remote/Messages/DbmClose.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmClose", DbmClose, IMessage)

bool DbmClose::serialize(Serializer& s)
{
	return true;
}

	}
}
