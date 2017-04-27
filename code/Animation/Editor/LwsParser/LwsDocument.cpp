/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/LwsParser/LwsDocument.h"
#include "Animation/Editor/LwsParser/LwsGroup.h"
#include "Animation/Editor/LwsParser/LwsValue.h"
#include "Core/RefArray.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/StringReader.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LwsDocument", LwsDocument, Object)

Ref< LwsDocument > LwsDocument::parse(IStream* stream)
{
	AnsiEncoding encoding;
	StringReader reader(stream, &encoding);
	std::wstring line;

	if (reader.readLine(line) < 0 || line != L"LWSC")
		return 0;
	if (reader.readLine(line) < 0 || (line != L"4" && line != L"5"))
		return 0;

	RefArray< LwsGroup > groupStack;
	groupStack.push_back(new LwsGroup(L""));

	while (reader.readLine(line) >= 0)
	{
		if (line.empty())
			continue;

		std::vector< std::wstring > pieces;
		Split< std::wstring >::any(line, L" \t", pieces);
		T_ASSERT (pieces.size() > 0);

		if (pieces[0] == L"{")
		{
			Ref< LwsGroup > group = new LwsGroup(pieces[1]);
			groupStack.back()->add(group);
			groupStack.push_back(group);
		}
		else if (pieces[0] == L"}")
		{
			if (groupStack.empty())
				return 0;
			groupStack.pop_back();
		}
		else
		{
			Ref< LwsValue > value = new LwsValue(pieces);
			groupStack.back()->add(value);
		}
	}

	if (groupStack.size() != 1)
		return 0;

	Ref< LwsDocument > document = new LwsDocument();
	document->m_rootGroup = groupStack.back();

	return document;
}

Ref< LwsGroup > LwsDocument::getRootGroup()
{
	return m_rootGroup;
}

	}
}
