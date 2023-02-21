/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	size_t ofs = 0;

	for (;;)
	{
		const size_t s = tmp.find(L"$(", ofs);
		if (s == std::string::npos)
			break;

		const size_t e = tmp.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		const std::wstring name = tmp.substr(s + 2, e - s - 2);

		if (env && env->has(name))
		{
			const std::wstring sub = replaceAll(env->get(name), L'\\', L'/');
			tmp = tmp.substr(0, s) + sub + tmp.substr(e + 1);
			ofs = s;
		}
		else if (OS::getInstance().getEnvironment(name, val))
		{
			const std::wstring sub = replaceAll(val, L'\\', L'/');
			tmp = tmp.substr(0, s) + sub + tmp.substr(e + 1);
			ofs = s;
		}
		else
		{
			const std::wstring sub =  L"$(" + name + L")";
			tmp = tmp.substr(0, s) + sub + tmp.substr(e + 1);
			ofs = s + sub.length();
		}
	}

	return tmp;
}

bool splitCommandLine(const std::wstring& commandLine, AlignedVector< std::wstring >& outArgv)
{
	size_t i = 0;
	while (i < commandLine.length())
	{
		StringOutputStream ss;

		// Trim whitespace.
		while (i < commandLine.length() && (commandLine[i] == L' ' || commandLine[i] == L'\t'))
			++i;
		if (i >= commandLine.length())
			break;

		// Parse argument until unescaped whitespace or end of line.
		while (i < commandLine.length())
		{
			if (commandLine[i] == L'\"' || commandLine[i] == L'\'')
			{
				size_t j = commandLine.find(commandLine[i], i + 1);
				if (j == commandLine.npos)
					return false;
				ss << commandLine.substr(i + 1, j - i - 1);
				i = j;
			}
			else if (commandLine[i] == L'\\')
			{
				if (++i >= commandLine.length())
					return false;
				ss << commandLine[i];
			}
			else if (commandLine[i] == L' ' || commandLine[i] == L'\t')
				break;
			else
				ss << commandLine[i];
			++i;
		}

		outArgv.push_back(ss.str());
	}
	return true;
}

}
