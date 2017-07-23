/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Conditions/Indentation.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Indentation", Indentation, Condition)

void Indentation::check(const Path& fileName, const Source& source, bool isHeader, OutputStream& report) const
{
	const std::vector< Source::Line >& lines = source.getUncommentedLines();

	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		const std::wstring& line = lines[i].text;
		for (uint32_t j = 0; j < uint32_t(line.length()); ++j)
		{
			if (line[j] == L' ')
			{
				report << L"Space used for indentation at " << lines[i].line << Endl;
				break;
			}
			if (line[j] != L'\t' && line[j] != L' ')
				break;
		}
	}

	for (uint32_t i = 0; i < uint32_t(lines.size()); ++i)
	{
		const std::wstring& line = lines[i].text;

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
				report << L"Too many braces on line " << lines[i].line << Endl;

			//const wchar_t* kw[] =
			//{
			//	L"if",
			//	L"while",
			//	L"for",
			//	L"until",
			//	L"switch",
			//	L"namespace",
			//	L"class",
			//	L"struct"
			//};

			//for (uint32_t j = 0; j < sizeof_array(kw); ++j)
			//{
			//	if (line.find(kw[j]) != std::wstring::npos)
			//		report << L"Brace(s) on same line as keyword, line " << lines[i].line << Endl;
			//}
		}
	}
}
