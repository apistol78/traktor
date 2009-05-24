#include "Conditions/Indentation.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Indentation", Indentation, Condition)

void Indentation::check(const std::vector< std::wstring >& lines, bool isHeader, OutputStream& report) const
{
	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		const std::wstring& line = lines[i];
		for (uint32_t j = 0; j < uint32_t(line.length() - 1); ++j)
		{
			if (line[j] == L' ' && line[j + 1] != '*')
			{
				report << L"Space used for indentation at " << (i + 1) << Endl;
				break;
			}
			if (line[j] != L'\t' && line[j] != L' ')
				break;
		}
	}

	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		const std::wstring& line = lines[i];

		int32_t brace = 0;
		for (uint32_t j = 0; j < uint32_t(line.length()); ++j)
		{
			if (line[j] == L'{')
				++brace;
			if (line[j] == L'}')
				--brace;
		}

		if (brace != 0)
		{
			if (brace < -1 || brace > 1)
				report << L"Too many braces on line " << (i + 1) << Endl;

			const wchar_t* kw[] =
			{
				L"if",
				L"while",
				L"for",
				L"until",
				L"switch",
				L"namespace",
				L"class",
				L"struct"
			};

			for (uint32_t j = 0; j < sizeof_array(kw); ++j)
			{
				if (line.find(kw[j]) != std::wstring::npos)
					report << L"Brace(s) on same line as keyword, line " << (i + 1) << Endl;
			}
		}
	}
}
