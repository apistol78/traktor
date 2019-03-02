#include "Database/Events/EvtInstanceCommitted.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceCommitted", 0, EvtInstanceCommitted, EvtInstance)

EvtInstanceCommitted::EvtInstanceCommitted(const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
{
}

	}
}
