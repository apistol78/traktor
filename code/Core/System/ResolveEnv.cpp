/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Core/System/Environment.h"
#include "Core/System/ResolveEnv.h"

namespace traktor
{

std::wstring resolveEnv(const std::wstring& s, const Environment* env)
{
	std::wstring tmp = s;
	std::wstring val;

	for (;;)
	{
		size_t s = tmp.find(L"$(");
		if (s == std::string::npos)
			break;

		size_t e = tmp.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		std::wstring name = tmp.substr(s + 2, e - s - 2);

		if (env)
		{
			if (env->has(name))
			{
				tmp = tmp.substr(0, s) + replaceAll< std::wstring >(env->get(name), L'\\', L'/') + tmp.substr(e + 1);
				continue;
			}
		}

		if (OS::getInstance().getEnvironment(name, val))
		{
			tmp = tmp.substr(0, s) + replaceAll< std::wstring >(val, L'\\', L'/') + tmp.substr(e + 1);
			continue;
		}

		tmp = tmp.substr(0, s) + tmp.substr(e + 1);
	}

	return tmp;
}

}
