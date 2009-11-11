#include "Ui/Associative.h"

namespace traktor
{
	namespace ui
	{

Associative::~Associative()
{
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
