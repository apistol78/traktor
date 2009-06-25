#include "Database/Remote/RemoteDatabase.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteDatabase", RemoteDatabase, IProviderDatabase)

bool RemoteDatabase::open(const net::Url& url)
{
	return false;
}

void RemoteDatabase::close()
{
}

IProviderBus* RemoteDatabase::getBus()
{
	return 0;
}

IProviderGroup* RemoteDatabase::getRootGroup()
{
	return 0;
}

	}
}
