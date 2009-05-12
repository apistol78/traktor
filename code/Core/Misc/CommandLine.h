#ifndef _traktor_CommandLine_H
#define _traktor_CommandLine_H

#include <vector>
#include <string>
#include "Core/Misc/StringUtils.h"
#include "Core/Misc/SplitString.h"
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
		inline Option(wchar_t name = 0, const std::wstring& value = L"")
		:	m_name(name)
		,	m_value(value)
		{
		}

		inline Option(const Option& src)
		:	m_name(src.m_name)
		,	m_value(src.m_value)
		{
		}

		inline wchar_t getName() const
		{
			return m_name;
		}

		inline int getInteger() const
		{
			return parseString< int >(m_value);
		}

		inline const std::wstring& getString() const
		{
			return m_value;
		}
			
	private:
		wchar_t m_name;
		std::wstring m_value;
	};

	inline CommandLine(int argc, const char** argv)
	:	m_invOpt(0)
	{
		m_file = mbstows(argv[0]);
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i][0] == L'-')
			{
				const char* value = (argv[i][2] == '=') ? &argv[i][3] : "";
				m_opts.push_back(Option(argv[i][1], mbstows(value)));
			}
			else 
				m_args.push_back(mbstows(argv[i]));
		}
	}

	inline CommandLine(const std::vector< std::wstring >& argv)
	:	m_invOpt(0)
	{
		m_file = argv[0];
		for (size_t i = 1; i < argv.size(); ++i)
		{
			if (argv[i][0] == L'-')
			{
				const wchar_t* value = (argv[i][2] == L'=') ? (argv[i].c_str() + 3) : L"";
				m_opts.push_back(Option(argv[i][1], value));
			}
			else
				m_args.push_back(argv[i]);
		}
	}

	inline CommandLine(const std::wstring& file, const std::wstring& args)
	:	m_file(file)
	,	m_invOpt(0)
	{
		size_t pos = 0;
		for (size_t i = 0; i <= args.length(); )
		{
			bool eol = i >= args.length();
			wchar_t ch = !eol ? args[i] : ' ';

			if (ch == ' ' || ch == '\t')
			{
				size_t len = i - pos;
				if (len > 0)
				{
					if (args[pos] == '-')
					{
						if (len >= 2 && args[pos + 2] == '=')
							m_opts.push_back(Option(
								args[pos + 1],
								args.substr(pos + 3, len - 2)
							));
						else
							m_opts.push_back(Option(
								args[pos + 1],
								L""
							));
					}
					else
					{
						m_args.push_back(args.substr(pos, len));
					}
				}
				pos = ++i;
			}
			else if (ch == '\"')
			{
				pos = ++i;
				while (i < args.length() && args[i] != '\"')
					++i;
				size_t len = i - pos;
				if (len > 0)
					m_args.push_back(args.substr(pos, len));
				pos = ++i;
			}
			else
				++i;
		}
	}

	inline const std::wstring& getFile() const
	{
		return m_file;
	}

	inline bool hasOption(wchar_t name) const
	{
		for (int i = 0; i < int(m_opts.size()); ++i)
		{
			if (m_opts[i].getName() == name)
				return true;
		}
		return false;
	}

	inline const Option& getOption(wchar_t name) const
	{
		for (int i = 0; i < int(m_opts.size()); ++i)
		{
			if (m_opts[i].getName() == name)
				return m_opts[i];
		}
		return m_invOpt;
	}

	inline int getCount() const
	{
		return int(m_args.size());
	}

	inline int getInteger(int index) const
	{
		return parseString< int >(m_args[index]);
	}

	inline std::wstring getString(int index) const
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
