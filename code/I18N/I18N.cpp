/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor::i18n
{

I18N& I18N::getInstance()
{
	static I18N* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new I18N();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void I18N::appendDictionary(const Dictionary* dictionary, bool overrideExisting)
{
	for (auto it : dictionary->get())
	{
		if (overrideExisting || !m_dictionary->has(it.first))
			m_dictionary->set(it.first, it.second);
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
