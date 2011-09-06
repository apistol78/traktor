#include "Database/Events/EvtInstanceCreated.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceCreated", 0, EvtInstanceCreated, EvtInstance)

EvtInstanceCreated::EvtInstanceCreated(const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
{
}

	}
}
