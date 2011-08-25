
#define T_XML_PARSER_THREAD 1

#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Xml/XmlPullParser.h"

#if T_XML_PARSER_THREAD
#	include "Core/Thread/JobManager.h"
#	include "Core/Thread/Signal.h"
#	include "Core/Thread/ThreadManager.h"
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
	uint8_t m_buf[4096];
	bool m_done;
	std::vector< wchar_t > m_cdata;
	XmlPullParser::Event m_eventQueue[2048];
	volatile uint32_t m_eventQueueHead;
	volatile uint32_t m_eventQueueTail;

#if T_XML_PARSER_THREAD
	Ref< Job > m_parseJob;
	Thread* m_threadJob;
	Signal m_eventQueueSignal;
#endif

	bool parse();

#if T_XML_PARSER_THREAD
	void parseJob();
#endif

	XmlPullParser::Event& allocEvent();

	void pushEvent();

	void pushCharacterData();

	static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts);

	static void XMLCALL endElement(void* userData, const XML_Char* name);

	static void XMLCALL characterData(void* userData, const XML_Char* s, int len);

	static int XMLCALL unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info);
};

XmlPullParserImpl::XmlPullParserImpl(IStream* stream)
:	m_stream(stream)
,	m_parser(0)
,	m_done(false)
,	m_eventQueueHead(0)
,	m_eventQueueTail(0)
#if T_XML_PARSER_THREAD
,	m_threadJob(0)
#endif
{
	m_parser = XML_ParserCreate(0);
	T_ASSERT_M (m_parser, L"Unable to create XML parser");

	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, startElement, endElement);
	XML_SetCharacterDataHandler(m_parser, characterData);
	XML_SetUnknownEncodingHandler(m_parser, unknownEncoding, 0);

	m_eventQueue[0].type = XmlPullParser::EtStartDocument;
	m_eventQueueTail++;

#if T_XML_PARSER_THREAD
	m_parseJob = JobManager::getInstance().add(makeFunctor(this, &XmlPullParserImpl::parseJob));
#endif
}

XmlPullParserImpl::~XmlPullParserImpl()
{
#if T_XML_PARSER_THREAD
	if (m_parseJob)
	{
		m_parseJob->wait();
		m_parseJob = 0;
	}
#endif
	if (m_parser)
		XML_ParserFree(m_parser);
}

bool XmlPullParserImpl::get(XmlPullParser::Event& outEvent)
{
#if T_XML_PARSER_THREAD

	// If queue empty; wait until event been queued.
	m_eventQueueSignal.reset();
	while (m_eventQueueHead == m_eventQueueTail)
	{
		if (!m_eventQueueSignal.wait(10000))
			return false;
		m_eventQueueSignal.reset();
	}

	outEvent = m_eventQueue[m_eventQueueHead];
	m_eventQueueHead = (m_eventQueueHead + 1) % sizeof_array(m_eventQueue);

#else
	while (m_eventQueueHead == m_eventQueueTail)
	{
		if (!parse())
			return false;
	}
	outEvent = m_eventQueue[m_eventQueueHead];
	m_eventQueueHead = (m_eventQueueHead + 1) % sizeof_array(m_eventQueue);
#endif
	return true;
}

bool XmlPullParserImpl::parse()
{
	if (!m_done)
	{
		int nread = m_stream->read(m_buf, sizeof(m_buf));
		T_ASSERT_M (nread >= 0, L"Unexpected out-of-data");

		m_done = nread < sizeof(m_buf);
		if (XML_Parse(m_parser, (const char*)m_buf, nread, m_done) == XML_STATUS_ERROR)
		{
			log::error << L"XML parse error" << Endl;

			XmlPullParser::Event& evt = allocEvent();
			evt.type = XmlPullParser::EtInvalid;
			pushEvent();
			
			return true;
		}
	}
	else
	{
		if (!m_parser)
			return false;

		XmlPullParser::Event& evt = allocEvent();
		evt.type = XmlPullParser::EtEndDocument;
		pushEvent();

		XML_ParserFree(m_parser);
		m_parser = 0;
	}
	return true;
}

#if T_XML_PARSER_THREAD
void XmlPullParserImpl::parseJob()
{
	m_threadJob = ThreadManager::getInstance().getCurrentThread();
	while (m_parser)
	{
		if (!parse())
			break;
	}
}
#endif

XmlPullParser::Event& XmlPullParserImpl::allocEvent()
{
#if T_XML_PARSER_THREAD
	// Check if event queue full; wait until event has been consumed.
	int32_t tail = (m_eventQueueTail + 1) % sizeof_array(m_eventQueue);
	while (tail == m_eventQueueHead)
		m_threadJob->sleep(100);
#endif

	XmlPullParser::Event& evt = m_eventQueue[m_eventQueueTail];
	evt.type = XmlPullParser::EtInvalid;
	evt.value.clear();
	evt.attr.clear();
	return evt;
}

void XmlPullParserImpl::pushEvent()
{
#if T_XML_PARSER_THREAD
	m_eventQueueTail = (m_eventQueueTail + 1) % sizeof_array(m_eventQueue);
	m_eventQueueSignal.set();
#else
	m_eventQueueTail = (m_eventQueueTail + 1) % sizeof_array(m_eventQueue);
#endif
}

void XmlPullParserImpl::pushCharacterData()
{
	size_t len = m_cdata.size();
	if (!len)
		return;

	const wchar_t* ss = &m_cdata[0];
	const wchar_t* es = &m_cdata[len - 1];

	while ((*ss == ' ' || *ss == '\t' || *ss == 10) && ss <= es)
		++ss;

	while ((*es == ' ' || *es == '\t' || *es == 10) && ss <= es)
		--es;

	if (ss <= es)
	{
		XmlPullParser::Event& evt = allocEvent();
		evt.type = XmlPullParser::EtText;
		evt.value = std::wstring(ss, es + 1);
		pushEvent();
	}

	m_cdata.resize(0);
}

void XMLCALL XmlPullParserImpl::startElement(void* userData, const XML_Char* name, const XML_Char** atts)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);

	pp->pushCharacterData();

	XmlPullParser::Event& evt = pp->allocEvent();
	evt.type = XmlPullParser::EtStartElement;
	evt.value = xmltows(name);

	for (int i = 0; atts[i]; i += 2)
		evt.attr.push_back(std::make_pair(xmltows(atts[i]), xmltows(atts[i + 1])));

	pp->pushEvent();
}

void XMLCALL XmlPullParserImpl::endElement(void* userData, const XML_Char* name)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);

	pp->pushCharacterData();

	XmlPullParser::Event& evt = pp->allocEvent();
	evt.type = XmlPullParser::EtEndElement;
	evt.value = xmltows(name);

	pp->pushEvent();
}

void XMLCALL XmlPullParserImpl::characterData(void* userData, const XML_Char* s, int len)
{
	XmlPullParserImpl* pp = reinterpret_cast< XmlPullParserImpl* >(userData);
	T_ASSERT (pp);
	T_ASSERT (len > 0);

	std::wstring ws = xmltows(s, &s[len]);
	pp->m_cdata.insert(pp->m_cdata.end(), ws.begin(), ws.end());
}

int XMLCALL XmlPullParserImpl::unknownEncoding(void* userData, const XML_Char* name, XML_Encoding* info)
{
	// Map Windows encoding as iso-8859-1; not completely accurat.
	// @fixme Should use IEncoding classes from Core.
	if (compareIgnoreCase< std::wstring >(xmltows(name), L"Windows-1252") == 0)
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
