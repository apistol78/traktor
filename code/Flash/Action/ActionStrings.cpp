/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionStrings.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

static const std::string c_empty("");

		}

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

const std::string& ActionStrings::operator [] (uint32_t id) const
{
	for (std::map< std::string, uint32_t >::const_iterator i = m_strings.begin(); i != m_strings.end(); ++i)
	{
		if (id == i->second)
			return i->first;
	}
	return c_empty;
}

	}
}
