#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Pipeline/AgentStatus.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentStatus", 0, AgentStatus, ISerializable)

AgentStatus::AgentStatus()
:	m_result(false)
{
}

AgentStatus::AgentStatus(const Guid& buildGuid, bool result)
:	m_buildGuid(buildGuid)
,	m_result(result)
{
}

const Guid& AgentStatus::getBuildGuid() const
{
	return m_buildGuid;
}

bool AgentStatus::getResult() const
{
	return m_result;
}

void AgentStatus::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"buildGuid", m_buildGuid);
	s >> Member< bool >(L"result", m_result);
}

	}
}
