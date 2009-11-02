#include "Database/Remote/Server/RemoteDatabaseService.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{
	
T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.RemoteDatabaseService", RemoteDatabaseService, net::IService)	

RemoteDatabaseService::RemoteDatabaseService()
:	m_port(0)
{
}

RemoteDatabaseService::RemoteDatabaseService(const std::wstring& host, uint16_t port)
:	m_host(host)
,	m_port(port)
{
}

const std::wstring& RemoteDatabaseService::getHost() const
{
	return m_host;
}

uint16_t RemoteDatabaseService::getPort() const
{
	return m_port;
}

std::wstring RemoteDatabaseService::getDescription() const
{
	return L"Remote database at \"" + m_host + L"\"";
}

bool RemoteDatabaseService::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< uint16_t >(L"port", m_port);
	return true;
}
	
	}
}
