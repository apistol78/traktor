#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Test/CaseXmlDocument.h"

namespace traktor::xml::test
{
	namespace
	{

const char c_validXml[] =
{
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	"<root>"
	"<child1>foo</child1>"
	"<child2/>"
	"<child3 attr=\"bar\"/>"
	"<child4 attr=\"foo\"/>"
	"</root>"
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.xml.test.CaseXmlDocument", 0, CaseXmlDocument, traktor::test::Case)

void CaseXmlDocument::run()
{
	MemoryStream stream((void*)c_validXml, std::strlen(c_validXml), true, false);
	xml::Document doc;
	CASE_ASSERT(doc.loadFromStream(&stream));
	CASE_ASSERT_EQUAL(doc.getDocumentElement()->getName(), L"root");
}

}
