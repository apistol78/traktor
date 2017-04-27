/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Associative.h"

namespace traktor
{
	namespace ui
	{

Associative::~Associative()
{
}

void Associative::removeAllData()
{
	m_data.clear();
}

void Associative::setData(const std::wstring& key, Object* data)
{
	if (data)
		m_data[key] = data;
	else
	{
		// Remove entry from map instead of just setting it to null.
		std::map< std::wstring, Ref< Object > >::iterator i = m_data.find(key);
		if (i != m_data.end())
			m_data.erase(i);
	}
}

Ref< Object > Associative::getData(const std::wstring& key) const
{
	std::map< std::wstring, Ref< Object > >::const_iterator i = m_data.find(key);
	return (i != m_data.end()) ? i->second.ptr() : 0;
}

	}
}
