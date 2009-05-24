#include <Core/Misc/StringUtils.h>
#include "Conditions/SingleEmptyLine.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SingleEmptyLine", SingleEmptyLine, Condition)

void SingleEmptyLine::check(const std::vector< std::wstring >& lines, bool isHeader, OutputStream& report) const
{
	bool previousEmpty = bool(trim(lines[0]).length() == 0);
	for (uint32_t i = 1; i < uint32_t(lines.size()); ++i)
	{
		bool currentEmpty = bool(trim(lines[i]).length() == 0);
		if (previousEmpty && currentEmpty)
			report << L"Multiple consecutive white lines at " << i << Endl;
		previousEmpty = currentEmpty;
	}
}
