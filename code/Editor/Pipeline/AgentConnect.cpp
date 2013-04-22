#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/Pipeline/AgentConnect.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentConnect", 0, AgentConnect, ISerializable)

AgentConnect::AgentConnect()
:	m_databasePort(0)
,	m_streamServerPort(0)
{
}

AgentConnect::AgentConnect(
	const Guid& sessionId,
	const PropertyGroup* settings,
	const std::wstring& host,
	uint16_t databasePort,
	uint16_t streamServerPort
)
:	m_sessionId(sessionId)
,	m_settings(settings)
,	m_host(host)
,	m_databasePort(databasePort)
,	m_streamServerPort(streamServerPort)
{
}

const Guid& AgentConnect::getSessionId() const
{
	return m_sessionId;
}

const PropertyGroup* AgentConnect::getSettings() const
{
	return m_settings;
}

const std::wstring& AgentConnect::getHost() const
{
	return m_host;
}

uint16_t AgentConnect::getDatabasePort() const
{
	return m_databasePort;
}

uint16_t AgentConnect::getStreamServerPort() const
{
	return m_streamServerPort;
}

bool AgentConnect::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"sessionId", m_sessionId);
	s >> MemberRef< const PropertyGroup >(L"settings", m_settings);
	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< uint16_t >(L"databasePort", m_databasePort);
	s >> Member< uint16_t >(L"streamServerPort", m_streamServerPort);
	return true;
}

	}
}
