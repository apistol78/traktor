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
	
	MemoryStream stream((void*)c_validXml, sizeof(c_validXml), true, false);
	if (!doc.loadFromStream(&stream))
	{
		failed("XML load from stream failed");
		return;
	}

	CASE_ASSERT_EQUAL(doc.getDocumentElement()->getName(), L"root");
}

}
