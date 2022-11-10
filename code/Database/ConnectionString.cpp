/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Database/ConnectionString.h"

namespace traktor::db
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

		std::wstring key = trim(i->substr(0, p));
		std::wstring value = trim(i->substr(p + 1));

		m_values.insert(std::make_pair(key, value));
	}
}

bool ConnectionString::have(const std::wstring& key) const
{
	return m_values.find(key) != m_values.end();
}

void ConnectionString::set(const std::wstring& key, const std::wstring& value)
{
	if (!value.empty())
		m_values[key] = value;
	else
		m_values.erase(key);
}

std::wstring ConnectionString::get(const std::wstring& key) const
{
	const auto i = m_values.find(key);
	return i != m_values.end() ? i->second : L"";
}

std::wstring ConnectionString::format() const
{
	StringOutputStream ss;
	for (auto i = m_values.begin(); i != m_values.end(); ++i)
	{
		if (i != m_values.begin())
			ss << L";";
		ss << i->first << L"=" << i->second;
	}
	return ss.str();
}

}
