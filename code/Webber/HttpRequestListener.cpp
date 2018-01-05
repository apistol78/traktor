#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Html/Attribute.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Net/Url.h"
#include "Net/Http/HttpRequest.h"
#include "Webber/HttpRequestListener.h"
#include "Webber/IApplication.h"
#include "Webber/Utilities.h"
#include "Webber/Widget.h"

// Resources
#include "Resources/Client.h"
#include "Resources/Stylesheet.h"

namespace traktor
{
	namespace wbr
	{
		namespace
		{
		
int32_t getNodeChildCount(const html::Node* e)
{
	int32_t count = 0;
	for (auto c = e->getFirstChild(); c != nullptr; c = c->getNextSibling())
		++count;
	return count;
}

void compareHtmlNode(const html::Node* left, const html::Node* right, const std::function< void(const html::Node*, const html::Node*) >& mismatch)
{
	if (&type_of(left) != &type_of(right))
	{
		mismatch(left, right);
		return;
	}

	if (is_a< html::Element >(left))
	{
		auto le = checked_type_cast< const html::Element* >(left);
		auto re = checked_type_cast< const html::Element* >(right);

		if (le->getName() != re->getName())
		{
			mismatch(left, right);
			return;
		}

		auto la = le->getFirstAttribute();
		auto ra = re->getFirstAttribute();

		while (la && ra)
		{
			if (la->getName() != ra->getName())
			{
				mismatch(left, right);
				return;
			}
			if (la->getValue() != ra->getValue())
			{
				mismatch(left, right);
				return;
			}
			la = la->getNext();
			ra = ra->getNext();
		}
		if (la != nullptr || ra != nullptr)
		{
			mismatch(left, right);
			return;
		}
	}

	if (is_a< html::Text >(left))
	{
		auto lt = checked_type_cast< const html::Text* >(left);
		auto rt = checked_type_cast< const html::Text* >(right);
		if (lt->getValue() != rt->getValue())
		{
			mismatch(left, right);
			return;
		}
	}

	if (getNodeChildCount(left) != getNodeChildCount(right))
	{
		mismatch(left, right);
		return;
	}

	auto lc = left->getFirstChild();
	auto rc = right->getFirstChild();

	while (lc != nullptr && rc != nullptr)
	{
		compareHtmlNode(lc, rc, mismatch);
		lc = lc->getNextSibling();
		rc = rc->getNextSibling();
	}
}

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.HttpRequestListener", HttpRequestListener, net::HttpServer::IRequestListener);

HttpRequestListener::HttpRequestListener(
	const std::function< Ref< IApplication >() >& newapp,
	bool verbose
)
:	m_newapp(newapp)
,	m_verbose(verbose)
{
}

int32_t HttpRequestListener::httpClientRequest(net::HttpServer* server, const net::HttpRequest* request, OutputStream& os, Ref< IStream >& outStream, bool& outCache, std::wstring& inoutSession)
{
	if (m_verbose)
		log::info << L"Got client request resource = \"" << request->getResource() << L"\", session = \"" << inoutSession << L"\"" << Endl;

	const std::wstring& resource = request->getResource();

	if (resource == L"/")
	{
		Guid sid(inoutSession);

		if (sid.isNull())
		{
			sid = Guid::create();
			inoutSession = sid.format();
		}

		auto& session = m_sessions[sid];

		if (!session.app)
		{
			session.app = m_newapp();
			if (session.app)
			{
				if (!session.app->create())
					return 404;
			}
			else
				return 404;
		}

		if (!session.doc)
		{
			session.doc = buildDocument(session.app);
			if (!session.doc)
				return 404;
		}

		session.doc->toString(os);
	}
	else if (startsWith< std::wstring >(resource, L"/event?"))
	{
		Guid sid(inoutSession);

		auto si = m_sessions.find(sid);
		if (si == m_sessions.end())
			return 404;

		auto& s = si->second;
		T_FATAL_ASSERT (s.app);
		T_FATAL_ASSERT (s.doc);

		std::wstring kvs = resource.substr(resource.find(L'?') + 1);

		// Parse GET encoded key/value pairs.
		std::map< std::wstring, std::wstring > kvm;
		StringSplit< std::wstring > ss(kvs, L"&");
		for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			const std::wstring& kv = *i;
			size_t ii = kv.find(L'=');
			if (ii != kv.npos)
			{
				std::wstring key = kv.substr(0, ii);
				std::wstring value =  net::Url::decodeString(kv.substr(ii + 1));
				kvm[key] = value;
			}
		}

		int32_t wid = parseString< int32_t >(kvm[L"wid"], -1);
		int32_t waction = parseString< int32_t >(kvm[L"wa"], -1);

		if (wid >= 0 && waction >= 0)
			s.app->getRootWidget()->consume(wid, waction);

		auto doc = buildDocument(s.app);

		os << L"{" << Endl;
		os << IncreaseIndent;

		compareHtmlNode(
			s.doc->getDocumentElement(),
			doc->getDocumentElement(),
			[&](const html::Node* left, const html::Node* right) {
				auto p = dynamic_type_cast< const html::Element* >(right->getParent());
				if (p != nullptr)
				{
					const auto id = p->getAttribute(L"id");
					if (id != nullptr)
					{
						StringOutputStream ss;
						p->innerHTML(ss);
						os << L"\"" << id->getValue() << L"\": \"" << escapeJson(ss.str()) << L"\"," << Endl;

						if (m_verbose)
							log::info << L"\"" << id->getValue() << L"\": \"" << escapeJson(ss.str()) << L"\"," << Endl;
					}
				}
			}
		);

		os << L"\"_eod_\": 0" << Endl;

		os << DecreaseIndent;
		os << L"}" << Endl;

		s.doc = doc;
	}
	else if (resource == L"/Client.js")
	{
		outStream = new MemoryStream(c_ResourceClient, sizeof(c_ResourceClient));
	}
	else if (resource == L"/Stylesheet.css")
	{
		outStream = new MemoryStream(c_ResourceStylesheet, sizeof(c_ResourceStylesheet));
	}
	else
	{
		Guid sid(inoutSession);

		auto si = m_sessions.find(sid);
		if (si == m_sessions.end())
			return 404;

		auto& s = si->second;
		T_FATAL_ASSERT (s.app);

		outStream = s.app->resolve(resource);

		if (!outStream)
		{
			log::error << L"Unable to resolve resource \"" << resource << L"\"." << Endl;
			return 404;
		}
	}

	return 200;
}

Ref< html::Document > HttpRequestListener::buildDocument(const IApplication* app) const
{
	Ref< html::Document > doc = new html::Document();

	Ref< html::Element > ehtml = new html::Element(L"html");
	doc->setDocumentElement(ehtml);

	Ref< html::Element > ehead = new html::Element(L"head");
	ehtml->addChild(ehead);

	Ref< html::Element > emeta = new html::Element(L"meta");
	emeta->setAttribute(L"charset", L"UTF-8");
	ehead->addChild(emeta);

	Ref< html::Element > escript1 = new html::Element(L"script");
	escript1->setAttribute(L"type", L"text/javascript");
	escript1->setAttribute(L"src", L"Client.js");
	ehead->addChild(escript1);

	Ref< html::Element > elink1 = new html::Element(L"link");
	elink1->setAttribute(L"rel", L"stylesheet");
	elink1->setAttribute(L"href", L"Stylesheet.css");
	ehead->addChild(elink1);

	Ref< html::Element > escript2 = new html::Element(L"script");
	escript2->setAttribute(L"type", L"text/javascript");
	escript2->setAttribute(L"src", L"highlight.pack.js");
	ehead->addChild(escript2);

	Ref< html::Element > elink2 = new html::Element(L"link");
	elink2->setAttribute(L"rel", L"stylesheet");
	elink2->setAttribute(L"href", L"styles/default.css");
	ehead->addChild(elink2);

	Ref< html::Element > escript3 = new html::Element(L"script");
	escript3->setAttribute(L"type", L"text/javascript");
	escript3->setAttribute(L"src", L"/ace/src-noconflict/ace.js");
	escript3->setAttribute(L"charset", L"utf-8");
	ehead->addChild(escript3);

	Ref< html::Element > ebody = new html::Element(L"body");
	ehtml->addChild(ebody);

	if (app)
		app->getRootWidget()->build(ebody);

	return doc;
}

	}
}
