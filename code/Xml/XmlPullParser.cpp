/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Xml/XmlPullParser.h"

#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

#include <sstream>

#define XML_STATIC
#include "expat.h"

namespace traktor::xml
{
namespace
{

const Utf8Encoding c_utf8enc;

std::wstring xmltows(const XML_Char* xmlstr, const XML_Char* term)
{
	const size_t len = (size_t)(term - xmlstr);
	return mbstows(c_utf8enc, std::string_view(xmlstr, len));
}

std::wstring xmltows(const XML_Char* xmlstr)
{
	return mbstows((const char*)xmlstr);
}

}

class XmlPullParserImpl
{
public:
	explicit XmlPullParserImpl(IStream* stream, const std::wstring& name);

	~XmlPullParserImpl();

	bool get(XmlPullParser::Event& outEvent);

private:
	Ref< IStream > m_stream;
	std::wstring m_name;
	XML_Parser m_parser = 0;
	uint8_t m_buf[4096];
	bool m_done = false;
	AlignedVector< wchar_t > m_cdata;
	int32_t m_cdataMode = 0;
	XmlPullParser::Event m_eventQueue[1024];
	uint32_t m_eventQueueHead = 0;
	uint32_t m_eventQueueTail = 0;

	bool parse();

	XmlPullParser::Event* allocEvent();

	void pushEvent();

	void pushCharacterData();

	static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts);

	static void XMLCALL endElement(void* userData, const XML_Char* name);

	static void XMLCALL characterData(void* userData, const XML_Char* s, int len);

	static void XMLCALL startCdataSectionData(void* userData);

	static void XMLCALL endCdataSectionData(void* userData);

	static int XMLCALL unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info);
};

XmlPullParserImpl::XmlPullParserImpl(IStream* stream, const std::wstring& name)
	: m_stream(stream)
	, m_name(name)
{
	m_parser = XML_ParserCreate(nullptr);
	T_ASSERT_M(m_parser, L"Unable to create XML parser");

	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, startElement, endElement);
	XML_SetCharacterDataHandler(m_parser, characterData);
	XML_SetCdataSectionHandler(m_parser, startCdataSectionData, endCdataSectionData);
	XML_SetUnknownEncodingHandler(m_parser, unknownEncoding, nullptr);

	m_eventQueue[0].type = XmlPullParser::EventType::StartDocument;
	m_eventQueueTail++;
}

XmlPullParserImpl::~XmlPullParserImpl()
{
	if (m_parser)
		XML_ParserFree(m_parser);
}

bool XmlPullParserImpl::get(XmlPullParser::Event& outEvent)
{
	while (m_eventQueueHead == m_eventQueueTail)
		if (!parse())
			return false;
	outEvent = m_eventQueue[m_eventQueueHead];
	m_eventQueueHead = (m_eventQueueHead + 1) % sizeof_array(m_eventQueue);
	return true;
}

bool XmlPullParserImpl::parse()
{
	if (!m_done)
	{
		const int64_t nread = m_stream->read(m_buf, sizeof(m_buf));
		if (nread < 0)
		{
			log::error << L"Unexpected out-of-data in XML parser (" << m_name << L")." << Endl;
			return false;
		}

		m_done = nread < sizeof(m_buf);
		if (XML_Parse(m_parser, (const char*)m_buf, (int)nread, m_done) == XML_STATUS_ERROR)
		{
			XML_Size line = XML_GetCurrentLineNumber(m_parser);
			log::error << L"XML parse error at line " << (int32_t)line << L" (" << m_name << L")." << Endl;

			XmlPullParser::Event* evt = allocEvent();
			if (!evt)
				return false;

			evt->type = XmlPullParser::EventType::Invalid;
			pushEvent();
			return true;
		}
	}
	else
	{
		if (!m_parser)
			return false;

		XmlPullParser::Event* evt = allocEvent();
		if (!evt)
			return false;

		evt->type = XmlPullParser::EventType::EndDocument;
		pushEvent();

		XML_ParserFree(m_parser);
		m_parser = nullptr;
	}
	return true;
}

XmlPullParser::Event* XmlPullParserImpl::allocEvent()
{
	auto& evt = m_eventQueue[m_eventQueueTail];
	evt.type = XmlPullParser::EventType::Invalid;
	evt.value.clear();
	evt.attr.clear();
	return &evt;
}

void XmlPullParserImpl::pushEvent()
{
	m_eventQueueTail = (m_eventQueueTail + 1) % sizeof_array(m_eventQueue);
	T_ASSERT(m_eventQueueTail != m_eventQueueHead);
}

void XmlPullParserImpl::pushCharacterData()
{
	const size_t len = m_cdata.size();
	if (!len)
		return;

	const wchar_t* ss = &m_cdata[0];
	const wchar_t* es = &m_cdata[len - 1];

	XmlPullParser::Event* evt = allocEvent();
	if (evt)
	{
		evt->type = XmlPullParser::EventType::Text;
		evt->value = std::wstring(ss, es + 1);
		pushEvent();
	}

	m_cdata.resize(0);
}

void XMLCALL XmlPullParserImpl::startElement(void* userData, const XML_Char* name, const XML_Char** atts)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT(pp);

	pp->pushCharacterData();

	XmlPullParser::Event* evt = pp->allocEvent();
	if (evt)
	{
		evt->type = XmlPullParser::EventType::StartElement;
		evt->value = xmltows(name);

		for (int32_t i = 0; atts[i]; i += 2)
			evt->attr.push_back(std::make_pair(xmltows(atts[i]), xmltows(atts[i + 1])));

		pp->pushEvent();
	}
}

void XMLCALL XmlPullParserImpl::endElement(void* userData, const XML_Char* name)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT(pp);

	pp->pushCharacterData();

	XmlPullParser::Event* evt = pp->allocEvent();
	if (evt)
	{
		evt->type = XmlPullParser::EventType::EndElement;
		evt->value = xmltows(name);
		pp->pushEvent();
	}
}

void XMLCALL XmlPullParserImpl::characterData(void* userData, const XML_Char* s, int len)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT(pp);
	T_ASSERT(len > 0);

	const std::wstring ws = xmltows(s, &s[len]);
	const wchar_t* ss = &ws[0];
	const wchar_t* es = &ws[len];
	pp->m_cdata.insert(pp->m_cdata.end(), ss, es);
}

void XMLCALL XmlPullParserImpl::startCdataSectionData(void* userData)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	pp->m_cdataMode++;
}

void XMLCALL XmlPullParserImpl::endCdataSectionData(void* userData)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	pp->m_cdataMode--;
}

int XMLCALL XmlPullParserImpl::unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info)
{
	// Map Windows encoding as iso-8859-1; not completely accurat.
	// @fixme Should use IEncoding classes from Core.
	if (compareIgnoreCase(xmltows(name), L"Windows-1252") == 0)
	{
		for (int32_t i = 0; i < 256; ++i)
			info->map[i] = i;

		info->data = nullptr;
		info->convert = nullptr;
		info->release = nullptr;

		return XML_STATUS_OK;
	}
	return XML_STATUS_ERROR;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlPullParser", XmlPullParser, Object)

XmlPullParser::XmlPullParser(IStream* stream, const std::wstring& name)
	: m_impl(new XmlPullParserImpl(stream, name))
	, m_pushed(0)
{
}

XmlPullParser::~XmlPullParser()
{
	delete m_impl;
}

XmlPullParser::EventType XmlPullParser::next()
{
	if (m_pushed > 0)
	{
		m_pushed--;
		return m_event.type;
	}

	if (m_event.type == EventType::EndDocument)
		return m_event.type;

	if (!m_impl)
		return EventType::Invalid;

	if (!m_impl->get(m_event))
	{
		delete m_impl;
		m_impl = nullptr;
		return EventType::Invalid;
	}

	return m_event.type;
}

void XmlPullParser::push()
{
	m_pushed++;
}

const XmlPullParser::Event& XmlPullParser::getEvent() const
{
	return m_event;
}

}
