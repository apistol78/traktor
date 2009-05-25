#include <Core/Misc/StringUtils.h>
#include "Conditions/IncludeOrder.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"IncludeOrder", IncludeOrder, Condition)

void IncludeOrder::check(const Source& source, bool isHeader, OutputStream& report) const
{
	const std::vector< Source::Line >& lines = source.getOriginalLines();
	uint32_t ltc = 0, htc = 0;

	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		std::wstring line = trim(lines[i].text);
		if (line.empty())
			continue;

		if (line[0] != L'#')
			continue;

		line = trim(line.substr(1));
		if (!startsWith(line, L"include"))
			continue;

		if (line.length() < 9)
		{
			report << L"Incorrect include statement at " << lines[i].line << Endl;
			continue;
		}

		if (line[8] == L'<')
		{
			if (htc > 0)
				report << L"#include <...> after #include \"...\" at line " << lines[i].line << Endl;
			++ltc;
		}
		else if (line[8] == L'\"')
			++htc;
		else
			report << L"Incorrect include statement at " << lines[i].line << Endl;
	}
}
