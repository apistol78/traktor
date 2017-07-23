/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/String.h>
#include "Conditions/SingleEmptyLine.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SingleEmptyLine", SingleEmptyLine, Condition)

void SingleEmptyLine::check(const Path& fileName, const Source& source, bool isHeader, OutputStream& report) const
{
	const std::vector< Source::Line >& lines = source.getOriginalLines();
	bool previousEmpty = bool(trim(lines[0].text).length() == 0);
	for (uint32_t i = 1; i < uint32_t(lines.size()); ++i)
	{
		bool currentEmpty = bool(trim(lines[i].text).length() == 0);
		if (previousEmpty && currentEmpty)
			report << L"Multiple consecutive white lines at " << lines[i].line << Endl;
		previousEmpty = currentEmpty;
	}
}
