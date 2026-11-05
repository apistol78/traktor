/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cwchar>
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Html/Attribute.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Net/Http/HttpRequest.h"
#include "Webber/HttpRequestListener.h"
#include "Webber/IApplication.h"
#include "Webber/Widget.h"

// Resources
#include "Resources/Client.h"
#include "Resources/Stylesheet.h"

namespace traktor::wbr
{
	namespace
	{

const size_t c_maxSessions = 1000;
const double c_sessionTimeout = 30.0 * 60.0;	//!< Seconds of inactivity before a session is removed.
const int32_t c_maxEventSize = 16 * 1024 * 1024;	//!< Events carry uploaded files.

std::wstring getContentType(const std::wstring& uri)
{
	const size_t p = uri.find_last_of(L"./");
	const std::wstring extension = (p != uri.npos && uri[p] == L'.') ? toLower(uri.substr(p + 1)) : L"";
	if (extension == L"css")
		return L"text/css; charset=utf-8";
	else if (extension == L"js")
		return L"text/javascript; charset=utf-8";
	else if (extension == L"png")
		return L"image/png";
	else if (extension == L"jpg" || extension == L"jpeg")
		return L"image/jpeg";
	else if (extension == L"webp")
		return L"image/webp";
	else if (extension == L"gif")
		return L"image/gif";
	else
		return L"application/octet-stream";
}

std::wstring decodeValue(const std::wstring& text)
{
	// Percent-decode into UTF-8 bytes.
	std::string bytes;
	for (size_t i = 0; i < text.length(); )
	{
		if (text[i] == L'%' && i + 2 < text.length())
		{
			bytes.push_back(char(std::wcstol(text.substr(i + 1, 2).c_str(), nullptr, 16)));
			i += 3;
		}
		else
			bytes.push_back(char(text[i++]));
	}

	// Decode UTF-8; stop at a truncated sequence.
	const Utf8Encoding encoding;
	const uint8_t* in = reinterpret_cast< const uint8_t* >(bytes.data());
	std::wstring out;
	for (size_t i = 0; i < bytes.length(); )
	{
		wchar_t ch;
		const int32_t r = encoding.translate(&in[i], int32_t(std::min< size_t >(IEncoding::MaxEncodingSize, bytes.length() - i)), ch);
		if (r <= 0)
			break;

		// Keep only characters valid in XML 1.0 text; control characters are never meaningful input.
		const uint32_t c = uint32_t(ch);
		if (c == 0x09 || c == 0x0a || c == 0x0d || (c >= 0x20 && c <= 0xd7ff) || (c >= 0xe000 && c <= 0xfffd) || (c >= 0x10000 && c <= 0x10ffff))
			out.push_back(ch);

		i += r;
	}
	return out;
}

std::wstring escapeJson(const std::wstring& text)
{
	StringOutputStream ss;
	for (auto c : text)
	{
		switch (c)
		{
		case L'\t':
			ss << L"\\t";
			break;
		case L'\n':
			ss << L"\\n";
			break;
		case L'\r':
			ss << L"\\r";
			break;
		case L'\"':
			ss << L"\\\"";
			break;
		case L'\\':
			ss << L"\\\\";
			break;
		default:
			if (c >= 0x20)
				ss << c;
			else
				ss << L' ';
			break;
		}
	}
	return ss.str();
}

int32_t getNodeChildCount(const html::Node* node)
{
	int32_t count = 0;
	for (auto c = node->getFirstChild(); c != nullptr; c = c->getNextSibling())
		++count;
	return count;
}

void addChangedElement(const html::Node* node, AlignedVector< const html::Element* >& outChanged)
{
	// Closest element with an id, starting at node; client can only patch elements it can look up.
	for (auto n = node; n != nullptr; n = n->getParent())
	{
		auto e = dynamic_type_cast< const html::Element* >(n);
		if (e != nullptr && e->hasAttribute(L"id"))
		{
			if (std::find(outChanged.begin(), outChanged.end(), e) == outChanged.end())
				outChanged.push_back(e);
			return;
		}
	}
}

void compareHtmlNode(const html::Node* left, const html::Node* right, AlignedVector< const html::Element* >& outChanged)
{
	if (&type_of(left) != &type_of(right))
	{
		addChangedElement(right->getParent(), outChanged);
		return;
	}

	if (is_a< html::Element >(left))
	{
		auto le = checked_type_cast< const html::Element* >(left);
		auto re = checked_type_cast< const html::Element* >(right);

		if (le->getName() != re->getName())
		{
			addChangedElement(right->getParent(), outChanged);
			return;
		}

		auto la = le->getFirstAttribute();
		auto ra = re->getFirstAttribute();

		while (la && ra)
		{
			if (la->getName() != ra->getName() || la->getValue() != ra->getValue())
			{
				addChangedElement(right->getParent(), outChanged);
				return;
			}
			la = la->getNext();
			ra = ra->getNext();
		}
		if (la != nullptr || ra != nullptr)
		{
			addChangedElement(right->getParent(), outChanged);
			return;
		}
	}

	if (is_a< html::Text >(left))
	{
		if (left->getValue() != right->getValue())
		{
			addChangedElement(right, outChanged);
			return;
		}
	}

	if (getNodeChildCount(left) != getNodeChildCount(right))
	{
		addChangedElement(right, outChanged);
		return;
	}

	auto lc = left->getFirstChild();
	auto rc = right->getFirstChild();

	while (lc != nullptr && rc != nullptr)
	{
		compareHtmlNode(lc, rc, outChanged);
		lc = lc->getNextSibling();
		rc = rc->getNextSibling();
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.HttpRequestListener", HttpRequestListener, net::HttpServer::IRequestListener)

HttpRequestListener::HttpRequestListener(
	const std::function< Ref< IApplication >() >& newapp,
	bool verbose
)
:	m_newapp(newapp)
,	m_verbose(verbose)
{
}

int32_t HttpRequestListener::httpClientRequest(
	net::HttpServer* server,
	const net::HttpRequest* request,
	IStream* clientStream,
	OutputStream& os,
	Ref< IStream >& outStream,
	std::map< std::wstring, std::wstring >& outHeaders,
	std::wstring& inoutSession
)
{
	const std::wstring& resource = request->getResource();

	if (m_verbose)
		log::info << L"Got client request resource = \"" << resource << L"\", session = \"" << inoutSession << L"\"" << Endl;

	// Content types are never sniffed and pages cannot be framed.
	outHeaders[L"X-Content-Type-Options"] = L"nosniff";
	outHeaders[L"Content-Security-Policy"] = L"frame-ancestors 'none'";

	if (resource == L"/")
	{
		// Unknown session ids are never adopted; a new session id is issued instead.
		IApplication* app = findSession(inoutSession);
		if (!app)
			app = createSession(inoutSession);
		if (!app)
			return 503;

		buildDocument(app)->toString(os);
		outHeaders[L"Content-Type"] = L"text/html; charset=utf-8";
		outHeaders[L"Cache-Control"] = L"no-cache";
	}
	else if (resource == L"/event" && clientStream != nullptr)
	{
		// Cross-site requests cannot carry a custom header without a CORS preflight, which is never granted.
		if (!request->hasValue(L"X-Webber"))
			return 403;

		if (parseString< int32_t >(request->getValue(L"Content-Length")) > c_maxEventSize)
			return 413;

		IApplication* app = findSession(inoutSession);
		if (!app)
			return 404;

		// Read form encoded body; values are percent-encoded so every byte is ASCII.
		std::wstring body;
		uint8_t buffer[4096];
		for (int64_t n; (n = clientStream->read(buffer, sizeof(buffer))) > 0; )
			body.append(buffer, buffer + n);

		// Input values are keyed by element id.
		int32_t wid = -1;
		int32_t waction = -1;
		bool navigate = false;
		std::wstring location;
		std::map< int32_t, std::wstring > values;
		for (const auto& kv : StringSplit< std::wstring >(body, L"&"))
		{
			const size_t p = kv.find(L'=');
			if (p == kv.npos)
				continue;

			const std::wstring key = kv.substr(0, p);
			const std::wstring value = decodeValue(kv.substr(p + 1));

			if (key == L"wid")
				wid = parseString< int32_t >(value, -1);
			else if (key == L"wa")
				waction = parseString< int32_t >(value, -1);
			else if (key == L"nav")
			{
				navigate = true;
				location = value;
			}
			else if (startsWith(key, L"_"))
				values[parseString< int32_t >(key.substr(1), -1)] = value;
		}

		// Baseline is what the client currently shows, including its input values.
		app->getRootWidget()->consumeValues(values);
		Ref< html::Document > baseline = buildDocument(app);

		if (navigate)
			app->navigate(location);
		else if (wid >= 0 && waction >= 0)
			app->getRootWidget()->consume(wid, waction);

		Ref< html::Document > doc = buildDocument(app);

		AlignedVector< const html::Element* > changed;
		compareHtmlNode(baseline->getDocumentElement(), doc->getDocumentElement(), changed);

		os << L"{" << Endl;
		os << IncreaseIndent;

		for (auto element : changed)
		{
			StringOutputStream ss;
			element->innerHTML(ss);

			const std::wstring id = element->getAttribute(L"id")->getValue();
			os << L"\"" << id << L"\": \"" << escapeJson(ss.str()) << L"\"," << Endl;

			if (m_verbose)
				log::info << L"Updated element \"" << id << L"\"." << Endl;
		}

		// Client records location changes in the browser's history.
		os << L"\"_location\": \"" << escapeJson(app->getLocation()) << L"\"," << Endl;
		os << L"\"_eod_\": 0" << Endl;

		os << DecreaseIndent;
		os << L"}" << Endl;

		outHeaders[L"Content-Type"] = L"application/json; charset=utf-8";
		outHeaders[L"Cache-Control"] = L"no-cache";
	}
	else if (resource == L"/Client.js")
	{
		outStream = new MemoryStream(c_ResourceClient, sizeof(c_ResourceClient));
		outHeaders[L"Content-Type"] = L"text/javascript; charset=utf-8";
	}
	else if (resource == L"/Stylesheet.css")
	{
		outStream = new MemoryStream(c_ResourceStylesheet, sizeof(c_ResourceStylesheet));
		outHeaders[L"Content-Type"] = L"text/css; charset=utf-8";
	}
	else
	{
		IApplication* app = findSession(inoutSession);
		if (!app)
			return 404;

		outStream = app->resolve(resource);
		if (!outStream)
		{
			if (m_verbose)
				log::warning << L"Unable to resolve resource \"" << resource << L"\"." << Endl;
			return 404;
		}

		outHeaders[L"Content-Type"] = getContentType(resource);
	}

	return 200;
}

IApplication* HttpRequestListener::findSession(const std::wstring& session)
{
	const Guid sid(session);
	auto it = sid.isValid() ? m_sessions.find(sid) : m_sessions.end();
	if (it == m_sessions.end())
		return nullptr;

	it->second.lastAccess = m_timer.getElapsedTime();
	return it->second.app;
}

IApplication* HttpRequestListener::createSession(std::wstring& outSession)
{
	const double now = m_timer.getElapsedTime();

	// Remove inactive sessions; refuse new sessions while still full.
	for (auto it = m_sessions.begin(); it != m_sessions.end(); )
	{
		if (now - it->second.lastAccess > c_sessionTimeout)
		{
			it->second.app->destroy();
			it = m_sessions.erase(it);
		}
		else
			++it;
	}

	if (m_sessions.size() >= c_maxSessions)
	{
		log::warning << L"Too many sessions; new session refused." << Endl;
		return nullptr;
	}

	Ref< IApplication > app = m_newapp();
	if (!app || !app->create())
		return nullptr;

	const Guid sid = Guid::create();
	m_sessions[sid] = { app, now };
	outSession = sid.format();
	return app;
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

	// Client navigates when the loaded URL names another location.
	Ref< html::Element > emetaLocation = new html::Element(L"meta");
	emetaLocation->setAttribute(L"name", L"webber-location");
	emetaLocation->setAttribute(L"content", app->getLocation());
	ehead->addChild(emetaLocation);

	Ref< html::Element > escript = new html::Element(L"script");
	escript->setAttribute(L"type", L"text/javascript");
	escript->setAttribute(L"src", L"Client.js");
	ehead->addChild(escript);

	Ref< html::Element > elink = new html::Element(L"link");
	elink->setAttribute(L"rel", L"stylesheet");
	elink->setAttribute(L"href", L"Stylesheet.css");
	ehead->addChild(elink);

	const std::wstring styleSheet = app->getStyleSheet();
	if (!styleSheet.empty())
	{
		Ref< html::Element > elinkApp = new html::Element(L"link");
		elinkApp->setAttribute(L"rel", L"stylesheet");
		elinkApp->setAttribute(L"href", styleSheet);
		ehead->addChild(elinkApp);
	}

	Ref< html::Element > ebody = new html::Element(L"body");
	ehtml->addChild(ebody);

	app->getRootWidget()->build(ebody);
	return doc;
}

}
