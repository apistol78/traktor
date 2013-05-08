#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtGroupRenamed.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtGroupRenamed", 0, EvtGroupRenamed, IEvent)

EvtGroupRenamed::EvtGroupRenamed()
{
}

EvtGroupRenamed::EvtGroupRenamed(const std::wstring& name, const std::wstring& previousPath)
:	m_name(name)
,	m_previousPath(previousPath)
{
}

const std::wstring& EvtGroupRenamed::getName() const
{
	return m_name;
}

const std::wstring& EvtGroupRenamed::getPreviousPath() const
{
	return m_previousPath;
}

void EvtGroupRenamed::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"previousPath", m_previousPath);
}

	}
}
