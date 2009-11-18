
#define T_XML_PARSER_THREAD 1
#define T_XML_PARSER_THREAD_LOG 0

#include <sstream>
#include "Xml/XmlPullParser.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

#if T_XML_PARSER_THREAD
#	include "Core/Thread/JobManager.h"
#	include "Core/Thread/Signal.h"
#	include "Core/Thread/Semaphore.h"
#	include "Core/Thread/Acquire.h"
#endif

// On Linux and Apple we externally depend on Expat being part of the system.
#if !defined(linux) && !defined(__APPLE__)
#	define XML_STATIC
#	define XML_UNICODE
#	define XML_UNICODE_WCHAR_T
#endif

#include "expat.h"

namespace traktor
{
	namespace xml
	{
		namespace
		{

inline std::wstring xmltows(const XML_Char* xmlstr, const XML_Char* term)
{
#if defined(XML_UNICODE)
#	if defined(XML_UNICODE_WCHAR_T)
	T_ASSERT (sizeof(XML_Char) == sizeof(wchar_t));
	return std::wstring(xmlstr, term);
#	else
	std::wstringstream ss;
	for (const XML_Char* s = xmlstr; s != term; ++s)
		ss << wchar_t(*s);
	return ss.str();
#	endif
#else
	return mbstows(std::string(xmlstr, term));
#endif
}

inline std::wstring xmltows(const XML_Char* xmlstr)
{
#if defined(XML_UNICODE)
#	if defined(XML_UNICODE_WCHAR_T)
	T_ASSERT (sizeof(XML_Char) == sizeof(wchar_t));
	return xmlstr;
#	else
	std::wstringstream ss;
	for (const XML_Char* s = xmlstr; *s; ++s)
		ss << wchar_t(*s);
	return ss.str();
#	endif
#else
	return mbstows(xmlstr);
#endif
}

		}

class XmlPullParserImpl
{
public:
	XmlPullParserImpl(IStream* stream);

	~XmlPullParserImpl();
	
	bool get(XmlPullParser::Event& outEvent);

private:
	Ref< IStream > m_stream;
	XML_Parser m_parser;
	bool m_done;
	std::vector< wchar_t > m_cdata;
	std::list< XmlPullParser::Event > m_eventQueue;

#if T_XML_PARSER_THREAD
	Job m_parseJob;
	Signal m_eventQueueSignal;
	Semaphore m_eventQueueLock;
#	if T_XML_PARSER_THREAD_LOG
	uint32_t m_getCalled;
	uint32_t m_getStalled;
#	endif
#endif

	bool parse();

#if T_XML_PARSER_THREAD
	void parseJob();
#endif

	void pushCharacterData();

	void pushEvent(const XmlPullParser::Event& xmlEvent);

	static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts);

	static void XMLCALL endElement(void* userData, const XML_Char* name);

	static void XMLCALL characterData(void* userData, const XML_Char* s, int len);

	static int XMLCALL unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info);
};

XmlPullParserImpl::XmlPullParserImpl(IStream* stream)
:	m_stream(stream)
,	m_parser(0)
,	m_done(false)
#if T_XML_PARSER_THREAD_LOG
,	m_getCalled(0)
,	m_getStalled(0)
#endif
{
	m_parser = XML_ParserCreate(0);
	T_ASSERT_M (m_parser, L"Unable to create XML parser");

	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, startElement, endElement);
	XML_SetCharacterDataHandler(m_parser, characterData);
	XML_SetUnknownEncodingHandler(m_parser, unknownEncoding, 0);

	XmlPullParser::Event evt;
	evt.type = XmlPullParser::EtStartDocument;
	m_eventQueue.push_back(evt);

#if T_XML_PARSER_THREAD
	m_parseJob = makeFunctor(this, &XmlPullParserImpl::parseJob);
	JobManager::getInstance().add(m_parseJob);
#endif
}

XmlPullParserImpl::~XmlPullParserImpl()
{
#if T_XML_PARSER_THREAD
	m_parseJob.wait();
#	if T_XML_PARSER_THREAD_LOG
	log::info << L"XmlPullParserImpl, " << m_getCalled << " get called, " << m_getStalled << " stalled" << Endl;
#	endif
#endif
	if (m_parser)
		XML_ParserFree(m_parser);
}

bool XmlPullParserImpl::get(XmlPullParser::Event& outEvent)
{
#if T_XML_PARSER_THREAD
#	if T_XML_PARSER_THREAD_LOG
	++m_getCalled;
#	endif
	m_eventQueueSignal.reset();
	while (m_eventQueue.empty())
	{
#	if T_XML_PARSER_THREAD_LOG
		++m_getStalled;
#	endif
		if (!m_eventQueueSignal.wait(1000))
			return false;
		m_eventQueueSignal.reset();
	}
	{
		Acquire< Semaphore > __lock__(m_eventQueueLock);
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
	}
#else
	while (m_eventQueue.empty())
	{
		if (!parse())
			return false;
	}
#endif
	return true;
}

bool XmlPullParserImpl::parse()
{
	if (!m_done)
	{
		uint8_t buf[4096];

		int nread = m_stream->read(buf, sizeof(buf));
		T_ASSERT_M (nread >= 0, L"Unexpected out-of-data");

		m_done = nread < sizeof(buf);
		if (XML_Parse(m_parser, (const char*)buf, nread, m_done) == XML_STATUS_ERROR)
		{
			log::error << L"XML parse error" << Endl;
			return false;
		}
	}
	else
	{
		if (!m_parser)
			return false;

		XmlPullParser::Event evt;
		evt.type = XmlPullParser::EtEndDocument;
		pushEvent(evt);

		XML_ParserFree(m_parser);
		m_parser = 0;
	}
	return true;
}

#if T_XML_PARSER_THREAD
void XmlPullParserImpl::parseJob()
{
	while (m_parser)
	{
		if (!parse())
			break;
	}
}
#endif

void XmlPullParserImpl::pushCharacterData()
{
	if (m_cdata.empty())
		return;

	XmlPullParser::Event evt;
	evt.type = XmlPullParser::EtText;
	evt.value = trim(std::wstring(&m_cdata[0], m_cdata.size()));
	if (!evt.value.empty())
		pushEvent(evt);

	m_cdata.resize(0);
}

void XmlPullParserImpl::pushEvent(const XmlPullParser::Event& xmlEvent)
{
#if T_XML_PARSER_THREAD
	Acquire< Semaphore > __lock__(m_eventQueueLock);
#endif
	m_eventQueue.push_back(xmlEvent);
#if T_XML_PARSER_THREAD
	m_eventQueueSignal.set();
#endif
}

void XMLCALL XmlPullParserImpl::startElement(void* userData, const XML_Char* name, const XML_Char** atts)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);

	pp->pushCharacterData();

	XmlPullParser::Event evt;
	evt.type = XmlPullParser::EtStartElement;
	evt.value = xmltows(name);

	for (int i = 0; atts[i]; i += 2)
		evt.attr.push_back(std::make_pair(xmltows(atts[i]), xmltows(atts[i + 1])));

	pp->pushEvent(evt);
}

void XMLCALL XmlPullParserImpl::endElement(void* userData, const XML_Char* name)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);

	pp->pushCharacterData();

	XmlPullParser::Event evt;
	evt.type = XmlPullParser::EtEndElement;
	evt.value = xmltows(name);

	pp->pushEvent(evt);
}

void XMLCALL XmlPullParserImpl::characterData(void* userData, const XML_Char* s, int len)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);

	std::wstring ws = xmltows(s, &s[len]);
	pp->m_cdata.insert(pp->m_cdata.end(), ws.begin(), ws.end());
}

int XMLCALL XmlPullParserImpl::unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info)
{
	// Map Windows encoding as iso-8859-1; not completely accurat.
	// @fixme Should use IEncoding classes from Core.
	if (compareIgnoreCase(xmltows(name), L"Windows-1252") == 0)
	{
		for (int i = 0; i < 256; ++i)
			info->map[i] = i;

		info->data = 0;
		info->convert = 0;
		info->release = 0;

		return XML_STATUS_OK;
	}
	return XML_STATUS_ERROR;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlPullParser", XmlPullParser, Object)

XmlPullParser::XmlPullParser(IStream* stream)
:	m_impl(new XmlPullParserImpl(stream))
,	m_pushed(0)
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

	if (m_event.type == EtEndDocument)
		return m_event.type;

	if (!m_impl)
		return EtInvalid;

	if (!m_impl->get(m_event))
	{
		delete m_impl; m_impl = 0;
		return EtInvalid;
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
}
