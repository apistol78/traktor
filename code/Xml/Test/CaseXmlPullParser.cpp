#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Xml/XmlPullParser.h"
#include "Xml/Test/CaseXmlPullParser.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.xml.test.CaseXmlPullParser", 0, CaseXmlPullParser, traktor::test::Case)

void CaseXmlPullParser::run()
{
	MemoryStream stream((void*)c_validXml, strlen(c_validXml), true, false);
	xml::XmlPullParser parser(&stream);

	CASE_ASSERT(parser.getEvent().type == xml::XmlPullParser::EventType::Invalid);
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartDocument);

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartElement);
	CASE_ASSERT(parser.getEvent().value == L"root");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartElement);
	CASE_ASSERT(parser.getEvent().value == L"child1");
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::Text);
	CASE_ASSERT(parser.getEvent().value == L"foo");
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndElement);
	CASE_ASSERT(parser.getEvent().value == L"child1");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartElement);
	CASE_ASSERT(parser.getEvent().value == L"child2");
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndElement);
	CASE_ASSERT(parser.getEvent().value == L"child2");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartElement);
	CASE_ASSERT(parser.getEvent().value == L"child3");
	CASE_ASSERT(parser.getEvent().attr.size() == 1);
	CASE_ASSERT(parser.getEvent().attr[0].first == L"attr");
	CASE_ASSERT(parser.getEvent().attr[0].second == L"bar");
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndElement);
	CASE_ASSERT(parser.getEvent().value == L"child3");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::StartElement);
	CASE_ASSERT(parser.getEvent().value == L"child4");
	CASE_ASSERT(parser.getEvent().attr.size() == 1);
	CASE_ASSERT(parser.getEvent().attr[0].first == L"attr");
	CASE_ASSERT(parser.getEvent().attr[0].second == L"foo");
	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndElement);
	CASE_ASSERT(parser.getEvent().value == L"child4");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndElement);
	CASE_ASSERT(parser.getEvent().value == L"root");

	CASE_ASSERT(parser.next() == xml::XmlPullParser::EventType::EndDocument);
}

}
