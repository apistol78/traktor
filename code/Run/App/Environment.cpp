#include "Run/App/Environment.h"

namespace traktor
{
	namespace run
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.Environment", Environment, Object)

Environment::Environment()
{
}

Environment::Environment(const OS::envmap_t& env)
:	m_env(env)
{
}

void Environment::set(const std::wstring& key, const std::wstring& value)
{
	m_env[key] = value;
}

std::wstring Environment::get(const std::wstring& key) const
{
	OS::envmap_t::const_iterator i = m_env.find(key);
	return i != m_env.end() ? i->second : L"";
}

const OS::envmap_t& Environment::envmap() const
{
	return m_env;
}

	}
}
