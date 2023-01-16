/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Dictionary.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor::i18n
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

void Dictionary::remove(const std::wstring& id)
{
	m_map.erase(id);
}

bool Dictionary::get(const std::wstring& id, std::wstring& outText) const
{
	auto it = m_map.find(id);
	if (it != m_map.end())
	{
		outText = it->second;
		return true;
	}
	else
		return false;
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
