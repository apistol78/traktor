#include "Database/Remote/Server/Configuration.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.Configuration", 0, Configuration, ISerializable)

uint16_t Configuration::getListenPort() const
{
	return m_listenPort;
}

std::wstring Configuration::getDatabaseManifest(const std::wstring& name) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_databaseManifests.find(name);
	return i != m_databaseManifests.end() ? i->second : L"";
}

bool Configuration::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"listenPort", m_listenPort);
	s >> MemberStlMap< std::wstring, std::wstring >(L"databaseManifests", m_databaseManifests);
	return true;
}

	}
}
