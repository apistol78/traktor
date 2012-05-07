#ifndef _traktor_CommandLine_H
#define _traktor_CommandLine_H

#include <cstring>
#include <string>
#include <vector>
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

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
		inline Option(const std::wstring& name = L"", const std::wstring& value = L"")
		:	m_name(name)
		,	m_value(value)
		{
		}

		inline Option(const Option& src)
		:	m_name(src.m_name)
		,	m_value(src.m_value)
		{
		}

		inline wchar_t getShortName() const
		{
			return m_name.length() == 1 ? m_name[0] : 0;
		}

		inline const std::wstring& getLongName() const
		{
			return m_name;
		}

		inline int32_t getInteger() const
		{
			return parseString< int32_t >(m_value);
		}

		inline const std::wstring& getString() const
		{
			return m_value;
		}

	private:
		std::wstring m_name;
		std::wstring m_value;
	};

	inline CommandLine(int argc, const char** argv)
	{
		m_file = mbstows(argv[0]);
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i][0] == L'-')
			{
				const char* value = std::strchr(argv[i], '=');
				if (value)
					m_opts.push_back(Option(
						mbstows(std::string(&argv[i][1], value)),
						mbstows(value + 1)
					));
				else
					m_opts.push_back(Option(
						mbstows(std::string(&argv[i][1])),
						L""
					));
			}
			else
				m_args.push_back(mbstows(argv[i]));
		}
	}

	inline CommandLine(const std::vector< std::wstring >& argv)
	{
		m_file = argv[0];
		for (size_t i = 1; i < argv.size(); ++i)
		{
			if (argv[i][0] == L'-')
			{
				const wchar_t* cs = argv[i].c_str() + 1;
				const wchar_t* value = wcschr(cs, L'=');
				if (value)
					m_opts.push_back(Option(
						std::wstring(cs, value),
						value + 1
					));
				else
					m_opts.push_back(Option(
						std::wstring(cs),
						L""
					));
			}
			else
				m_args.push_back(argv[i]);
		}
	}

	inline CommandLine(const std::wstring& file, const std::wstring& args)
	:	m_file(file)
	{
		std::vector< std::wstring > argv;

		size_t pos = 0;
		for (size_t i = 0; i <= args.length(); )
		{
			bool eol = i >= args.length();
			wchar_t ch = !eol ? args[i] : L' ';

			if (ch == L' ' || ch == L'\t')
			{
				size_t len = i - pos;
				if (len > 0)
					argv.push_back(args.substr(pos, len));
				pos = ++i;
			}
			else if (ch == L'\"')
			{
				pos = ++i;
				while (i < args.length() && args[i] != L'\"')
					++i;
				size_t len = i - pos;
				if (len > 0)
					argv.push_back(args.substr(pos, len));
				pos = ++i;
			}
			else
				++i;
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
						value + 1
					));
				else
					m_opts.push_back(Option(
						std::wstring(cs),
						L""
					));
			}
			else
				m_args.push_back(argv[i]);
		}
	}

	inline const std::wstring& getFile() const
	{
		return m_file;
	}

	inline bool hasOption(wchar_t shortName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName)
				return true;
		}
		return false;
	}

	inline const Option& getOption(wchar_t shortName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	inline bool hasOption(const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getLongName() == longName)
				return true;
		}
		return false;
	}

	inline const Option& getOption(const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getLongName() == longName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	inline bool hasOption(wchar_t shortName, const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
				return true;
		}
		return false;
	}

	inline const Option& getOption(wchar_t shortName, const std::wstring& longName) const
	{
		for (size_t i = 0; i < m_opts.size(); ++i)
		{
			if (m_opts[i].getShortName() == shortName || m_opts[i].getLongName() == longName)
				return m_opts[i];
		}
		return m_invOpt;
	}

	inline size_t getCount() const
	{
		return m_args.size();
	}

	inline int32_t getInteger(size_t index) const
	{
		return parseString< int32_t >(m_args[index]);
	}

	inline const std::wstring& getString(size_t index) const
	{
		return m_args[index];
	}

private:
	std::wstring m_file;
	std::vector< Option > m_opts;
	std::vector< std::wstring > m_args;
	Option m_invOpt;
};

}

#endif	// _traktor_CommandLine_H
