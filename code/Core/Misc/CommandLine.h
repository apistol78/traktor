#ifndef _traktor_CommandLine_H
#define _traktor_CommandLine_H

#include <cstring>
#include <string>
#include <vector>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

#include "Core/Log/Log.h"

namespace traktor
{

/*! \brief Helper class for command line arguments.
 * \ingroup Core
 *
 * Parse command line in the form of:
 * [argument] or -[option]=[value]
 */
class CommandLine
{
public:
	class Option
	{
		friend class CommandLine;

	public:
		Option(const std::wstring& name = L"", const std::wstring& value = L"")
		:	m_name(name)
		,	m_value(value)
		{
		}

		Option(const Option& src)
		:	m_name(src.m_name)
		,	m_value(src.m_value)
		{
		}

		wchar_t getShortName() const
		{
			return m_name.length() == 1 ? m_name[0] : 0;
		}

		const std::wstring& getLongName() const
		{
			return m_name;
		}

		int32_t getInteger() const
		{
			return parseString< int32_t >(m_value);
		}

		const std::wstring& getString() const
		{
			return m_value;
		}

	private:
		std::wstring m_name;
		std::wstring m_value;
	};

	explicit CommandLine(int argc, const char** argv)
	{
		m_file = mbstows(argv[0]);

#if !defined(_WIN32)
		StringOutputStream ss;
		for (int i = 1; i < argc; ++i)
		{
			if (i > 1)
				ss << L" ";
			ss << mbstows(argv[i]);
		}
		parse(ss.str());
#else
		for (int i = 1; i < argc; ++i)
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
#endif
	}

	explicit CommandLine(const std::vector< std::wstring >& argv)
	{
		m_file = argv[0];

#if !defined(_WIN32)
		StringOutputStream ss;
		for (size_t i = 1; i < argv.size(); ++i)
		{
			if (i > 1)
				ss << L" ";
			ss << argv[i];
		}
		parse(ss.str());
#else
		for (size_t i = 1; i < argv.size(); ++i)
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
#endif
	}

	explicit CommandLine(const std::wstring& file, const std::wstring& args)
	:	m_file(file)
	{
		parse(args);
	}

	const std::wstring& getFile() const
	{
		return m_file;
	}

	bool hasOption(wchar_t shortName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName)
				return true;
		}
		return false;
	}

	const Option& getOption(wchar_t shortName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	bool hasOption(const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getLongName() == longName)
				return true;
		}
		return false;
	}

	const Option& getOption(const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getLongName() == longName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	bool hasOption(wchar_t shortName, const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
				return true;
		}
		return false;
	}

	const Option& getOption(wchar_t shortName, const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	size_t getCount() const
	{
		return m_args.size();
	}

	int32_t getInteger(size_t index) const
	{
		return parseString< int32_t >(m_args[index]);
	}

	const std::wstring& getString(size_t index) const
	{
		return m_args[index];
	}

private:
	std::wstring m_file;
	std::vector< Option > m_opts;
	std::vector< std::wstring > m_args;
	Option m_invOpt;

	std::wstring trim(const std::wstring& s) const
	{
		if (s.size() >= 2 && s.front() == L'\"' && s.back() == L'\"')
			return s.substr(1, s.length() - 2);
		else
			return s;
	}

	void parse(const std::wstring& args)
	{
		std::vector< std::wstring > argv;
		wchar_t ch;

		for (size_t i = 0; i < args.length(); )
		{
			ch = args[i];
			if (ch == L' ' || ch == L'\t')
			{
				++i;
				continue;
			}

			size_t s = i;
			while (i < args.length())
			{
				ch = args[i];
				if (ch == L' ' || ch == '\t')
					break;

				++i;

				if (ch == L'\"')
				{
					while (i < args.length())
					{
						ch = args[i++];
						if (ch == L'\"')
							break;
					}
				}
			}

			if (i > s)
				argv.push_back(args.substr(s, i - s));
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
	}	
};

}

#endif	// _traktor_CommandLine_H
