/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.CommandLine", CommandLine, Object)

CommandLine::CommandLine(int argc, const char** argv)
{
	m_file = mbstows(argv[0]);
	parse(argc - 1, argv + 1);
}

CommandLine::CommandLine(int argc, const wchar_t** argv)
{
	m_file = argv[0];
	parse(argc - 1, argv + 1);
}

CommandLine::CommandLine(const std::wstring& file, const std::wstring& args)
:	m_file(file)
{
	parse(args);
}

const std::wstring& CommandLine::getFile() const
{
	return m_file;
}

bool CommandLine::hasOption(wchar_t shortName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getShortName() == shortName)
			return true;
	}
	return false;
}

const CommandLine::Option& CommandLine::getOption(wchar_t shortName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getShortName() == shortName)
			return m_opts[i];
	}
	return m_invOpt;
}

bool CommandLine::hasOption(const std::wstring& longName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getLongName() == longName)
			return true;
	}
	return false;
}

const CommandLine::Option& CommandLine::getOption(const std::wstring& longName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getLongName() == longName)
			return m_opts[i];
	}
	return m_invOpt;
}

bool CommandLine::hasOption(wchar_t shortName, const std::wstring& longName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
			return true;
	}
	return false;
}

const CommandLine::Option& CommandLine::getOption(wchar_t shortName, const std::wstring& longName) const
{
	for (size_t i = 0; i < m_opts.size(); ++i)
	{
		if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
			return m_opts[i];
	}
	return m_invOpt;
}

uint32_t CommandLine::getCount() const
{
	return (uint32_t)m_args.size();
}

int32_t CommandLine::getInteger(uint32_t index) const
{
	return parseString< int32_t >(m_args[index]);
}

const std::wstring& CommandLine::getString(uint32_t index) const
{
	return m_args[index];
}

std::wstring CommandLine::trim(const std::wstring& s) const
{
	size_t ln = s.length();
	if (ln >= 2)
	{
		if (s[0] == L'\"' && s[ln - 1] == L'\"')
			return s.substr(1, ln - 2);
	}
	return s;
}

bool CommandLine::parse(const std::wstring& args)
{
	AlignedVector< std::wstring > argv;

	size_t i = 0;
	while (i < args.length())
	{
		StringOutputStream ss;

		// Trim whitespace.
		while (i < args.length() && (args[i] == L' ' || args[i] == L'\t'))
			++i;
		if (i >= args.length())
			break;

		// Parse argument until unescaped whitespace or end of line.
		while (i < args.length())
		{
			if (args[i] == L'\"' || args[i] == L'\'')
			{
				size_t j = args.find(args[i], i + 1);
				if (j == args.npos)
					return false;
				ss << args.substr(i + 1, j - i - 1);
				i = j;
			}
			else if (args[i] == L'\\')
			{
				if (++i >= args.length())
					return false;
				ss << args[i];
			}
			else if (args[i] == L' ' || args[i] == L'\t')
				break;
			else
				ss << args[i];
			++i;
		}

		argv.push_back(ss.str());
	}

	for (size_t i = 0; i < argv.size(); ++i)
	{
		if (argv[i][0] == L'-')
		{
			const wchar_t* cs = argv[i].c_str() + 1;
			const wchar_t* value = wcschr(cs, L'=');
			if (value)
				m_opts.push_back(Option(
					std::wstring(cs, value),
					trim(value + 1)
				));
			else
				m_opts.push_back(Option(
					std::wstring(cs),
					L""
				));
		}
		else
			m_args.push_back(trim(argv[i]));
	}

	return true;
}

void CommandLine::parse(int argc, const char** argv)
{
	for (int i = 0; i < argc; ++i)
	{
		std::wstring a = mbstows(argv[i]);
		if (a[0] == L'-')
		{
			const wchar_t* cs = a.c_str() + 1;
			const wchar_t* value = wcschr(cs, L'=');
			if (value)
				m_opts.push_back(Option(
					std::wstring(cs, value),
					trim(value + 1)
				));
			else
				m_opts.push_back(Option(
					std::wstring(cs),
					L""
				));
		}
		else
			m_args.push_back(trim(a));
	}
}

void CommandLine::parse(int argc, const wchar_t** argv)
{
	for (int i = 0; i < argc; ++i)
	{
		std::wstring a = argv[i];
		if (a[0] == L'-')
		{
			const wchar_t* cs = a.c_str() + 1;
			const wchar_t* value = wcschr(cs, L'=');
			if (value)
				m_opts.push_back(Option(
					std::wstring(cs, value),
					trim(value + 1)
				));
			else
				m_opts.push_back(Option(
					std::wstring(cs),
					L""
				));
		}
		else
			m_args.push_back(trim(a));
	}
}

CommandLine::Option::Option(const std::wstring& name, const std::wstring& value)
:	m_name(name)
,	m_value(value)
{
}

CommandLine::Option::Option(const Option& src)
:	m_name(src.m_name)
,	m_value(src.m_value)
{
}

wchar_t CommandLine::Option::getShortName() const
{
	return m_name.length() == 1 ? m_name[0] : 0;
}

const std::wstring& CommandLine::Option::getLongName() const
{
	return m_name;
}

int32_t CommandLine::Option::getInteger() const
{
	return parseString< int32_t >(m_value);
}

const std::wstring& CommandLine::Option::getString() const
{
	return m_value;
}

}
