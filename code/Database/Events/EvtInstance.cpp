#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtInstance.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.EvtInstance", EvtInstance, IEvent)

EvtInstance::EvtInstance(const Guid& instanceGuid)
:	m_instanceGuid(instanceGuid)
{
}

const Guid& EvtInstance::getInstanceGuid() const
{
	return m_instanceGuid;
}

void EvtInstance::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"instanceGuid", m_instanceGuid);
}

	}
}
