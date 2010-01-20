#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Database/ConnectionString.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ConnectionString", ConnectionString, Object)

ConnectionString::ConnectionString(const std::wstring& connectionString)
{
	std::vector< std::wstring > kv;
	Split< std::wstring >::any(connectionString, L";,", kv);
	for (std::vector< std::wstring >::const_iterator i = kv.begin(); i != kv.end(); ++i)
	{
		size_t p = i->find(L'=');
		if (p == std::wstring::npos)
			continue;

		std::wstring key = toLower(trim(i->substr(0, p)));
		std::wstring value = trim(i->substr(p + 1));

		m_values.insert(std::make_pair(key, value));
	}
}

bool ConnectionString::have(const std::wstring& key) const
{
	return m_values.find(toLower(key)) != m_values.end();
}

std::wstring ConnectionString::get(const std::wstring& key) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_values.find(toLower(key));
	return i != m_values.end() ? i->second : L"";
}

	}
}
