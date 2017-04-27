/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "UnitTest/CaseXmlPullParser.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Xml/XmlPullParser.h"

namespace traktor
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

void CaseXmlPullParser::run()
{
	MemoryStream stream((void*)c_validXml, strlen(c_validXml), true, false);
	xml::XmlPullParser parser(&stream);

	CASE_ASSERT_EQUAL(parser.getEvent().type, xml::XmlPullParser::EtInvalid);
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartDocument);

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"root");
	log::info << parser.getEvent().value << Endl;

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child1");
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtText);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"foo");
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child1");

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child2");
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child2");

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child3");
	CASE_ASSERT_EQUAL(parser.getEvent().attr.size(), 1);
	CASE_ASSERT_EQUAL(parser.getEvent().attr[0].first, L"attr");
	CASE_ASSERT_EQUAL(parser.getEvent().attr[0].second, L"bar");
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child3");

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtStartElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child4");
	CASE_ASSERT_EQUAL(parser.getEvent().attr.size(), 1);
	CASE_ASSERT_EQUAL(parser.getEvent().attr[0].first, L"attr");
	CASE_ASSERT_EQUAL(parser.getEvent().attr[0].second, L"foo");
	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"child4");

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndElement);
	CASE_ASSERT_EQUAL(parser.getEvent().value, L"root");

	CASE_ASSERT_EQUAL(parser.next(), xml::XmlPullParser::EtEndDocument);
}

}
