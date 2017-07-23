/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/StringReader.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Misc/String.h>
#include "Source.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Source", Source, Object)

bool Source::create(IStream* file)
{
	StringReader sr(file, new Utf8Encoding());

	std::wstring text;
	for (uint32_t i = 1; sr.readLine(text) >= 0; ++i)
	{
		Line ol = { i, text };
		m_originalLines.push_back(ol);
	}

	bool blockComment = false;
	for (uint32_t i = 0; i < uint32_t(m_originalLines.size()); ++i)
	{
		const Line& line = m_originalLines[i];
		std::wstring text = line.text;

		if (!blockComment)
		{
			std::wstring::size_type j = text.find(L"//");
			if (j != std::wstring::npos)
			{
				text = trim(text.substr(0, j));
				if (text.length() <= 0)
					continue;
			}

			j = text.find(L"/*");
			if (j != std::wstring::npos)
			{
				blockComment = true;
				text = trim(text.substr(0, j));
				if (text.length() <= 0)
					continue;
			}

			Line ucl = { line.line, text };
			m_uncommentedLines.push_back(ucl);
		}
		else
		{
			std::wstring::size_type j = text.find(L"*/");
			if (j != std::wstring::npos)
			{
				blockComment = false;
				text = trim(text.substr(j + 2));
				if (text.length() > 0)
				{
					Line ucl = { line.line, text };
					m_uncommentedLines.push_back(ucl);
				}
			}
		}
	}

	return true;
}
