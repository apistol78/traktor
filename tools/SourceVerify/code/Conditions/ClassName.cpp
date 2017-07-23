/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/String.h>
#include <Core/Misc/WildCompare.h>
#include "Conditions/ClassName.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ClassName", ClassName, Condition)

void ClassName::check(const Path& fileName, const Source& source, bool isHeader, OutputStream& report) const
{
	if (!isHeader)
		return;

	const std::vector< Source::Line >& lines = source.getUncommentedLines();
	
	std::wstring expectedClassName = fileName.getFileNameNoExtension();
	bool foundMatchingClass = false;
	bool foundNonMatchingClass = false;

	WildCompare wc2(L"* class *");

	for (std::vector< Source::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
	{
		std::wstring text = trim(i->text);
		if (text.empty())
			continue;

		std::vector< std::wstring > pieces;
		if (wc2.match(text, WildCompare::CmCaseSensitive, &pieces))
		{
			std::wstring className = pieces[1];

			if (startsWith< std::wstring >(className, L"T_DLLCLASS "))
				className = className.substr(11);
			else if (startsWith< std::wstring >(className, L"T_DLLEXPORT "))
				className = className.substr(12);
			else if (startsWith< std::wstring >(className, L"T_NOVTABLE "))
				className = className.substr(11);
			
			std::wstring::size_type j = className.find(L':');
			if (j != std::wstring::npos)
				className = className.substr(0, j - 1);
			
			className = trim(className);

			if (className.find(L'_') != std::wstring::npos)
				report << L"Incorrect naming convention on class \"" << className << L"\" at line " << i->line << Endl;

			if (!endsWith< std::wstring >(className, L";"))
			{
				if (className == expectedClassName)
				{
					foundMatchingClass = true;
					break;
				}
				else
					foundNonMatchingClass = true;
			}
		}
	}

	if (foundNonMatchingClass && !foundMatchingClass)
		report << L"No class \"" << expectedClassName << L"\" found in header" << Endl;
}
