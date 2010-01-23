#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Database/Remote/Server/Configuration.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.Configuration", 0, Configuration, ISerializable)

Configuration::Configuration()
:	m_listenPort(33666)
{
}

void Configuration::setListenPort(uint16_t listenPort)
{
	m_listenPort = listenPort;
}

uint16_t Configuration::getListenPort() const
{
	return m_listenPort;
}

void Configuration::setConnectionString(const std::wstring& name, const std::wstring& connectionString)
{
	m_connectionStrings.insert(std::make_pair(name, connectionString));
}

std::wstring Configuration::getConnectionString(const std::wstring& name) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_connectionStrings.find(name);
	return i != m_connectionStrings.end() ? i->second : L"";
}

bool Configuration::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"listenPort", m_listenPort);
	s >> MemberStlMap< std::wstring, std::wstring >(L"connectionStrings", m_connectionStrings);
	return true;
}

	}
}
