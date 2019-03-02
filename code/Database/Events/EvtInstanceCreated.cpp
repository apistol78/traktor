#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtInstanceCreated.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceCreated", 0, EvtInstanceCreated, EvtInstance)

EvtInstanceCreated::EvtInstanceCreated(const std::wstring& groupPath, const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
,	m_groupPath(groupPath)
{
}

const std::wstring& EvtInstanceCreated::getGroupPath() const
{
	return m_groupPath;
}

void EvtInstanceCreated::serialize(ISerializer& s)
{
	EvtInstance::serialize(s);
	s >> Member< std::wstring >(L"groupPath", m_groupPath);
}

	}
}
