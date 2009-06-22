#include "Database/Local/Context.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Context", Context, Object)

Context::Context(bool preferBinary)
:	m_sessionGuid(Guid::create())
,	m_preferBinary(preferBinary)
{
}

const Guid& Context::getSessionGuid() const
{
	return m_sessionGuid;
}

bool Context::preferBinary() const
{
	return m_preferBinary;
}

	}
}
