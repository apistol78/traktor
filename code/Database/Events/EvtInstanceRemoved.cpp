#include "Database/Events/EvtInstanceRemoved.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceRemoved", 0, EvtInstanceRemoved, EvtInstance)

EvtInstanceRemoved::EvtInstanceRemoved(const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
{
}

	}
}
