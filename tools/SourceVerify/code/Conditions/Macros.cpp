#include <Core/Misc/StringUtils.h>
#include "Conditions/Macros.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Macros", Macros, Condition)

void Macros::check(const std::vector< std::wstring >& lines, bool isHeader, OutputStream& report) const
{
	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		if (isHeader && i == 1)
			continue;

		std::wstring line = trim(lines[i]);
		if (line.empty())
			continue;

		if (line[0] != L'#')
			continue;

		line = trim(line.substr(1));
		if (!startsWith(line, L"define"))
			continue;

		for (uint32_t j = 7; j < uint32_t(line.length()); ++j)
		{
			wchar_t ch = line[j];
			if (ch == L' ' || ch == L'\t' || ch == L'(')
				break;

			if (ch != L'_' && !(ch >= L'A' && ch <= L'Z') && !(ch >= L'0' && ch <= L'9'))
			{
				report << L"#define at " << (i + 1) << L" doesn't use correct macro naming convent" << Endl;
				break;
			}
		}
	}
}
