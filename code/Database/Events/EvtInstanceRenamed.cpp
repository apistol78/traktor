#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtInstanceRenamed.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceRenamed", 0, EvtInstanceRenamed, EvtInstance)

EvtInstanceRenamed::EvtInstanceRenamed()
:	EvtInstance(Guid())
{
}

EvtInstanceRenamed::EvtInstanceRenamed(const Guid& instanceGuid, const std::wstring& previousName)
:	EvtInstance(instanceGuid)
,	m_previousName(previousName)
{
}

const std::wstring& EvtInstanceRenamed::getPreviousName() const
{
	return m_previousName;
}

void EvtInstanceRenamed::serialize(ISerializer& s)
{
	EvtInstance::serialize(s);
	s >> Member< std::wstring >(L"previousName", m_previousName);
}

	}
}
