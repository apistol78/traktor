/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/WildCompare.h>
#include <Core/Misc/String.h>
#include <Core/Misc/Split.h>
#include "Conditions/StatementDisposition.h"
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"StatementDisposition", StatementDisposition, Condition)

void StatementDisposition::check(const Path& fileName, const Source& source, bool isHeader, OutputStream& report) const
{
	const std::vector< Source::Line >& lines = source.getUncommentedLines();
	std::vector< std::wstring > pieces;

	WildCompare wcif(L"if*(*)*");
	WildCompare wcfor(L"for*(*;*;*)*");
	WildCompare wcwhile(L"while*(*)*");
	WildCompare wcswitch(L"switch*(*)*");

	for (std::vector< Source::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
	{
		std::wstring text = trim(i->text);
		if (text.empty() || text[0] == L'#')
			continue;

		std::vector< std::wstring > statements;
		Split< std::wstring >::any(text, L";", statements);

		for (std::vector< std::wstring >::const_iterator j = statements.begin(); j != statements.end(); ++j)
		{
			std::wstring statement = trim(*j);
			if (statement.empty())
			{
				report << L"Empty statement at " << i->line << Endl;
				continue;
			}

			pieces.resize(0);
			if (wcif.match(statement, WildCompare::CmCaseSensitive, &pieces))
			{
				bool correct = true;

				correct &= bool(pieces[0] == L" ");
				//correct &= bool(trim(pieces[2]).length() == 0);

				if (!correct)
					report << L"Statement \"if\" not correctly formated at line " << i->line << Endl;
			}

			pieces.resize(0);
			if (wcfor.match(statement, WildCompare::CmCaseSensitive, &pieces))
			{
				bool correct = true;

				correct &= bool(pieces[0] == L" ");
				correct &= bool(pieces[1].length() == 0 || pieces[1][0] != L' ');
				correct &= bool(pieces[2].length() == 0 || pieces[2][0] == L' ');
				correct &= bool(pieces[3].length() == 0 || pieces[3][0] == L' ');
				//correct &= bool(trim(pieces[4]).length() == 0);

				if (!correct)
					report << L"Statement \"for\" not correctly formated at line " << i->line << Endl;
			}

			pieces.resize(0);
			if (wcwhile.match(statement, WildCompare::CmCaseSensitive, &pieces))
			{
				bool correct = true;

				correct &= bool(pieces[0] == L" ");
				//correct &= bool(trim(pieces[2]).length() == 0);

				if (!correct)
					report << L"Statement \"while\" not correctly formated at line " << i->line << Endl;
			}

			pieces.resize(0);
			if (wcswitch.match(statement, WildCompare::CmCaseSensitive, &pieces))
			{
				bool correct = true;

				correct &= bool(pieces[0] == L" ");
				//correct &= bool(trim(pieces[2]).length() == 0);

				if (!correct)
					report << L"Statement \"switch\" not correctly formated at line " << i->line << Endl;
			}
		}
	}
}
