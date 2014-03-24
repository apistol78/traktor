#ifndef Environment_H
#define Environment_H

#include <Core/System/OS.h>

class Environment : public traktor::Object
{
	T_RTTI_CLASS;

public:
	Environment();

	Environment(const traktor::OS::envmap_t& env);

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	const traktor::OS::envmap_t& envmap() const;

private:
	traktor::OS::envmap_t m_env;
};

#endif	// Environment_H
