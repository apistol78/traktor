/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <map>
#include "Core/Guid.h"
#include "Core/Timer/Timer.h"
#include "Net/Http/HttpServer.h"

namespace traktor::html
{

class Document;

}

namespace traktor::wbr
{

class IApplication;

/*! HTTP request listener serving one application instance per session.
 * \ingroup Webber
 *
 * Client events carry the values of all inputs; the document is rebuilt and
 * changed subtrees are sent back as JSON keyed by element id. The application's
 * location is sent along so browser back and forward can navigate to it.
 */
class HttpRequestListener : public net::HttpServer::IRequestListener
{
	T_RTTI_CLASS;

public:
	explicit HttpRequestListener(
		const std::function< Ref< IApplication >() >& newapp,
		bool verbose
	);

	virtual int32_t httpClientRequest(
		net::HttpServer* server,
		const net::HttpRequest* request,
		IStream* clientStream,
		OutputStream& os,
		Ref< IStream >& outStream,
		std::map< std::wstring, std::wstring >& outHeaders,
		std::wstring& inoutSession
	) override final;

private:
	struct Session
	{
		Ref< IApplication > app;
		double lastAccess;
	};

	std::function< Ref< IApplication >() > m_newapp;
	std::map< Guid, Session > m_sessions;
	Timer m_timer;
	bool m_verbose;

	/*! Application of an existing session, null if session is unknown. */
	IApplication* findSession(const std::wstring& session);

	/*! Create application in a new session; null when session limit is reached. */
	IApplication* createSession(std::wstring& outSession);

	Ref< html::Document > buildDocument(const IApplication* app) const;
};

}
