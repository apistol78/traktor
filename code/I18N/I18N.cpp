#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{
	namespace i18n
	{

I18N& I18N::getInstance()
{
	static I18N* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new I18N();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void I18N::appendDictionary(const Dictionary* dictionary, bool overrideExisting)
{
	const std::map< std::wstring, std::wstring >& map = dictionary->get();
	for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
	{
		if (overrideExisting || !m_dictionary->has(i->first))
			m_dictionary->set(i->first, i->second);
	}
}

Ref< Dictionary > I18N::getMasterDictionary()
{
	return m_dictionary;
}

std::wstring I18N::get(const std::wstring& id, const std::wstring& defaultText) const
{
	std::wstring result;
	return m_dictionary->get(id, result) ? result : defaultText;
}

void I18N::destroy()
{
	delete this;
}

I18N::I18N()
:	m_dictionary(new Dictionary())
{
}

	}
}
