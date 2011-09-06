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

bool EvtInstanceRenamed::serialize(ISerializer& s)
{
	if (!EvtInstance::serialize(s))
		return false;

	return s >> Member< std::wstring >(L"previousName", m_previousName);
}

	}
}
