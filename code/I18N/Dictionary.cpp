#include "I18N/Dictionary.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.i18n.Dictionary", 0, Dictionary, ISerializable)

bool Dictionary::has(const std::wstring& id) const
{
	return m_map.find(id) != m_map.end();
}

void Dictionary::set(const std::wstring& id, const std::wstring& text)
{
	m_map[id] = text;
}

bool Dictionary::get(const std::wstring& id, std::wstring& outText) const
{
	std::map< std::wstring, std::wstring >::const_iterator it = m_map.find(id);
	if (it == m_map.end())
		return false;

	outText = it->second;
	return true;
}

const std::map< std::wstring, std::wstring >& Dictionary::get() const
{
	return m_map;
}

void Dictionary::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"map", m_map);
}

	}
}
