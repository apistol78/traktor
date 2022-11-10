/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/System/Environment.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.Environment", 0, Environment, Object)

void Environment::set(const std::wstring& key, const std::wstring& value)
{
	m_env[key] = value;
}

void Environment::insert(const std::map< std::wstring, std::wstring >& env)
{
	m_env.insert(env.begin(), env.end());
}

bool Environment::has(const std::wstring& key) const
{
	auto it = m_env.find(key);
	return it != m_env.end();
}

std::wstring Environment::get(const std::wstring& key) const
{
	auto it = m_env.find(key);
	return it != m_env.end() ? it->second : L"";
}

const std::map< std::wstring, std::wstring >& Environment::get() const
{
	return m_env;
}

void Environment::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"env", m_env);
}

}
