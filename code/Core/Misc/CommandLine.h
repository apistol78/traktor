#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Helper class for command line arguments.
 * \ingroup Core
 *
 * Parse command line in the form of:
 * [argument] or -[option]=[value]
 */
class T_DLLCLASS CommandLine : public Object
{
	T_RTTI_CLASS;

public:
	class T_DLLCLASS Option
	{
		friend class CommandLine;

	public:
		Option(const std::wstring& name = L"", const std::wstring& value = L"");

		Option(const Option& src);

		wchar_t getShortName() const;

		const std::wstring& getLongName() const;

		int32_t getInteger() const;

		const std::wstring& getString() const;

	private:
		std::wstring m_name;
		std::wstring m_value;
	};

	explicit CommandLine(int argc, const char** argv);

	explicit CommandLine(const std::vector< std::wstring >& argv);

	explicit CommandLine(const std::wstring& file, const std::wstring& args);

	const std::wstring& getFile() const;

	bool hasOption(wchar_t shortName) const;

	const Option& getOption(wchar_t shortName) const;

	bool hasOption(const std::wstring& longName) const;

	const Option& getOption(const std::wstring& longName) const;

	bool hasOption(wchar_t shortName, const std::wstring& longName) const;

	const Option& getOption(wchar_t shortName, const std::wstring& longName) const;

	uint32_t getCount() const;

	int32_t getInteger(uint32_t index) const;

	const std::wstring& getString(uint32_t index) const;

private:
	std::wstring m_file;
	std::vector< Option > m_opts;
	std::vector< std::wstring > m_args;
	Option m_invOpt;

	std::wstring trim(const std::wstring& s) const;

	void parse(const std::wstring& args);
};

}

