#include "Flash/Action/ActionStrings.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionStrings", ActionStrings, Object)

ActionStrings::ActionStrings()
:	m_next(0)
{
}

uint32_t ActionStrings::operator [] (const std::string& str)
{
	std::map< std::string, uint32_t >::const_iterator i = m_strings.find(str);
	if (i != m_strings.end())
		return i->second;

	uint32_t id = m_next++;
	m_strings.insert(std::make_pair(str, id));

	return id;
}

std::string ActionStrings::operator [] (uint32_t id) const
{
	for (std::map< std::string, uint32_t >::const_iterator i = m_strings.begin(); i != m_strings.end(); ++i)
	{
		if (id == i->second)
			return i->first;
	}
	return "";
}

	}
}
