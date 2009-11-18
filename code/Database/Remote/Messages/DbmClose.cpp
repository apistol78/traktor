#include "Database/Remote/Messages/DbmClose.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmClose", DbmClose, IMessage)

bool DbmClose::serialize(ISerializer& s)
{
	return true;
}

	}
}
