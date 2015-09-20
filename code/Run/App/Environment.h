#ifndef traktor_run_Environment_H
#define traktor_run_Environment_H

#include "Core/System/OS.h"

namespace traktor
{
	namespace run
	{

class Environment : public Object
{
	T_RTTI_CLASS;

public:
	Environment();

	Environment(const OS::envmap_t& env);

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	const OS::envmap_t& envmap() const;

private:
	OS::envmap_t m_env;
};

	}
}

#endif	// traktor_run_Environment_H
