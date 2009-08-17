#include "Animation/Editor/LwsParser/LwsDocument.h"
#include "Animation/Editor/LwsParser/LwsGroup.h"
#include "Animation/Editor/LwsParser/LwsValue.h"
#include "Core/Heap/GcNew.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/StringReader.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LwsDocument", LwsDocument, Object)

LwsDocument* LwsDocument::parse(Stream* stream)
{
	AnsiEncoding encoding;
	StringReader reader(stream, &encoding);
	std::wstring line;

	if (reader.readLine(line) < 0 || line != L"LWSC")
		return 0;
	if (reader.readLine(line) < 0 || line != L"4")
		return 0;

	RefArray< LwsGroup > groupStack;
	groupStack.push_back(gc_new< LwsGroup >(L""));

	while (reader.readLine(line) >= 0)
	{
		if (line.empty())
			continue;

		std::vector< std::wstring > pieces;
		Split< std::wstring >::any(line, L" \t", pieces);
		T_ASSERT (pieces.size() > 0);

		if (pieces[0] == L"{")
		{
			Ref< LwsGroup > group = gc_new< LwsGroup >(pieces[1]);
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
			Ref< LwsValue > value = gc_new< LwsValue >(pieces);
			groupStack.back()->add(value);
		}
	}

	if (groupStack.size() != 1)
		return 0;

	Ref< LwsDocument > document = gc_new< LwsDocument >();
	document->m_rootGroup = groupStack.back();

	return document;
}

LwsGroup* LwsDocument::getRootGroup()
{
	return m_rootGroup;
}

	}
}
