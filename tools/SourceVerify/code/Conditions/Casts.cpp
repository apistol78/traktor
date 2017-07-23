/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/String.h>
#include "Conditions/Casts.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Casts", Casts, Condition)

void Casts::check(const Path& fileName, const Source& source, bool isHeader, OutputStream& report) const
{
	const wchar_t* castOperators[] =
	{
		L"static_cast",
		L"dynamic_cast",
		L"reinterpret_cast",
		L"const_cast",
		L"checked_type_cast",
		L"dynamic_type_cast"
	};

	const std::vector< Source::Line >& lines = source.getUncommentedLines();
	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		for (uint32_t j = 0; j < sizeof_array(castOperators); ++j)
		{
			std::wstring line = trim(lines[i].text);
			if (line.empty())
				continue;

			size_t p = line.find(castOperators[j]);
			if (p == std::wstring::npos)
				continue;

			size_t plt = line.find(L'<', p);
			size_t pgt = line.find(L'>', p);

			if (plt == line.npos && pgt == line.npos)
				continue;

			if (plt == line.npos || pgt == line.npos || pgt - plt < 3)
			{
				report << castOperators[j] << L" operation at " << lines[i].line << L" incorrectly formated, must be \"" << castOperators[j] << L"< ... >(...)" << Endl;
				continue;
			}

			if (plt - p != std::wstring(castOperators[j]).length())
			{
				report << castOperators[j] << L" operation at " << lines[i].line << L" incorrectly formated, must be \"" << castOperators[j] << L"< ... >(...)" << Endl;
				continue;
			}

			if (line[plt + 1] != L' ' || line[pgt - 1] != L' ')
			{
				report << castOperators[j] << L" operation at " << lines[i].line << L" incorrectly formated, must be \"" << castOperators[j] << L"< ... >(...)" << Endl;
				continue;
			}
		}
	}
}
