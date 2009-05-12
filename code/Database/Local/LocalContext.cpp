#include "Database/Local/LocalContext.h"
#include "Database/Local/FileDataAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalContext", LocalContext, Object)

LocalContext::LocalContext(bool binary)
:	m_sessionGuid(Guid::create())
,	m_dataAccess(gc_new< FileDataAccess >(binary))
{
}

const Guid& LocalContext::getSessionGuid() const
{
	return m_sessionGuid;
}

DataAccess* LocalContext::getDataAccess() const
{
	return m_dataAccess;
}

	}
}
