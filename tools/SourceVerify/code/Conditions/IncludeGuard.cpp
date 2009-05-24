#include <Core/Misc/StringUtils.h>
#include "Conditions/IncludeGuard.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"IncludeGuard", IncludeGuard, Condition)

void IncludeGuard::check(const std::vector< std::wstring >& lines, bool isHeader, OutputStream& report) const
{
	if (!isHeader)
		return;

	if (lines.size() < 3)
	{
		report << L"Not enough lines in header to fit proper include guard" << Endl;
		return;
	}

	std::wstring lockDefine = L"(Unknown)";

	if (startsWith(lines[0], L"#ifndef "))
		lockDefine = lines[0].substr(8);
	else
		report << L"First line doesn't start with \"#ifndef ...\"" << Endl;

	if (startsWith(lines[1], L"#define "))
	{
		if (lines[1].substr(8) != lockDefine)
			report << L"Second line include guard symbol mismatch, should be \"" << lockDefine << L"\"" << Endl;
	}
	else
		report << L"Second line doesn't start with \"#define ...\"" << Endl;

	if (startsWith(lines.back(), L"#endif\t// "))
	{
		if (lines.back().substr(10) != lockDefine)
			report << L"Last line include guard symbol mismatch, should be \"#endif<tab>// " << lockDefine << L"\"" << Endl;
	}
	else
		report << L"Last line doesn't start with \"#endif<tab>// ...\"" << Endl;
}
