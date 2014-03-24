#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"

namespace traktor
{

std::wstring resolveEnv(const std::wstring& s)
{
	std::wstring tmp = s;
	std::wstring env;

	for (;;)
	{
		size_t s = tmp.find(L"$(");
		if (s == std::string::npos)
			break;

		size_t e = tmp.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		std::wstring name = tmp.substr(s + 2, e - s - 2);

		if (OS::getInstance().getEnvironment(name, env))
			tmp = tmp.substr(0, s) + replaceAll< std::wstring >(env, L'\\', L'/') + tmp.substr(e + 1);
		else
			tmp = tmp.substr(0, s) + tmp.substr(e + 1);
	}

	return tmp;
}

}
