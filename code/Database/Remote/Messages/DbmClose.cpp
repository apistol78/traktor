#include "Database/Remote/Messages/DbmClose.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmClose", 0, DbmClose, IMessage)

void DbmClose::serialize(ISerializer& s)
{
}

	}
}
