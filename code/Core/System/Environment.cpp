/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/System/Environment.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Environment", Environment, Object)

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
	std::map< std::wstring, std::wstring >::const_iterator i = m_env.find(key);
	return i != m_env.end();
}

std::wstring Environment::get(const std::wstring& key) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_env.find(key);
	return i != m_env.end() ? i->second : L"";
}

const std::map< std::wstring, std::wstring >& Environment::get() const
{
	return m_env;
}

}
