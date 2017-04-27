/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseXmlDocument.h"
#include "Core/Io/MemoryStream.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace
	{

const wchar_t c_validXml[] =
{
	L"<?xml version=\"1.0\" encoding=\"utf-16\"?>"
	L"<!DOCTYPE doc["
	L"<!ENTITY myentity \"Hello world\">"
	L"]>"
	L"<root>"
	L"<!-- Comment -->"
	L"<child1>foo</child1>"
	L"<child2/>"
	L"<child3 attr=\"bar\"/>"
	L"<child4 attr=\"&myentity;\"/>"
	L"</root>"
};

	}

void CaseXmlDocument::run()
{
	xml::Document doc;
	
	if (!doc.loadFromFile(L"d:\\private\\dre\\traktor\\data\\Assets\\User\\branches.DAE"))
	{
		failed(L"XML load from stream failed");
		return;
	}

	CASE_ASSERT_EQUAL(doc.getDocumentElement()->getName(), L"root");
}

}
