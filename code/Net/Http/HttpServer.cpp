/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <list>
#include <string>
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Timer/Timer.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Http/HttpRequest.h"
#include "Net/Http/HttpServer.h"

namespace traktor::net
{
	namespace
	{

const int32_t c_maxConnections = 32;			//!< Keeps socket set within poll capacity and Windows FD_SETSIZE.
const size_t c_maxHeaderSize = 64 * 1024;
const int32_t c_maxContentLength = 16 * 1024 * 1024;
const double c_headerTimeout = 10.0;			//!< Seconds to receive complete header.
const double c_idleTimeout = 10.0;				//!< Seconds without data before connection is dropped.
const int32_t c_writeTimeout = 10000;			//!< Milliseconds per response write.

std::wstring decodeHeader(const std::string& header)
{
	// Decode UTF-8; stop at a truncated sequence.
	const uint8_t* in = reinterpret_cast< const uint8_t* >(header.data());
	std::wstring out;
	for (size_t i = 0; i < header.length(); )
	{
		wchar_t ch;
		const int32_t r = Utf8Encoding::getInstance()->translate(&in[i], int32_t(std::min< size_t >(IEncoding::MaxEncodingSize, header.length() - i)), ch);
		if (r <= 0)
			break;
		out.push_back(ch);
		i += r;
	}
	return out;
}

	}

/*! Collects requests from all connections as data arrives; only complete requests are dispatched.
 * Listener is always called from update, one request at a time.
 */
class HttpServerImpl : public Object
{
public:
	explicit HttpServerImpl(HttpServer* server)
	:	m_server(server)
	{
	}

	virtual ~HttpServerImpl()
	{
		destroy();
	}

	bool create(const SocketAddressIPv4& bind)
	{
		m_serverSocket = new TcpSocket();

		if (!m_serverSocket->bind(bind, true))
			return false;

		if (!m_serverSocket->listen())
			return false;

		return true;
	}

	void destroy()
	{
		for (auto& connection : m_connections)
			safeClose(connection.socket);

		m_connections.clear();
		m_listener = nullptr;

		safeClose(m_serverSocket);
	}

	int32_t getListenPort()
	{
		return dynamic_type_cast< net::SocketAddressIPv4* >(m_serverSocket->getLocalAddress())->getPort();
	}

	void setRequestListener(HttpServer::IRequestListener* listener)
	{
		m_listener = listener;
	}

	void update(int32_t duration)
	{
		const double until = m_timer.getElapsedTime() + duration / 1000.0;
		for (;;)
		{
			double now = m_timer.getElapsedTime();

			// Drop slow connections so they cannot hold on to connection slots.
			for (auto it = m_connections.begin(); it != m_connections.end(); )
			{
				if (now - it->received > c_idleTimeout || (!it->request && now - it->accepted > c_headerTimeout))
				{
					safeClose(it->socket);
					it = m_connections.erase(it);
				}
				else
					++it;
			}

			// Wait for new connection or data on pending connections.
			SocketSet sockets;
			if ((int32_t)m_connections.size() < c_maxConnections)
				sockets.add(m_serverSocket);
			for (const auto& connection : m_connections)
				sockets.add(connection.socket);

			SocketSet ready;
			const int32_t wait = std::clamp< int32_t >(int32_t((until - now) * 1000.0), 0, 1000);
			if (sockets.select(true, false, false, wait, ready) > 0)
			{
				now = m_timer.getElapsedTime();

				if (ready.contain(m_serverSocket))
				{
					Ref< TcpSocket > socket = m_serverSocket->accept();
					if (socket)
						m_connections.push_back({ socket, now, now });
				}

				for (auto it = m_connections.begin(); it != m_connections.end(); )
				{
					if (ready.contain(it->socket) && !receive(*it, now))
					{
						safeClose(it->socket);
						it = m_connections.erase(it);
					}
					else
						++it;
				}
			}

			if (m_timer.getElapsedTime() >= until)
				break;
		}
	}

private:
	struct Connection
	{
		Ref< TcpSocket > socket;
		double accepted;
		double received;
		std::string header;
		std::string body;
		Ref< HttpRequest > request;
		int32_t contentLength = 0;
	};

	HttpServer* m_server;
	Ref< TcpSocket > m_serverSocket;
	Ref< HttpServer::IRequestListener > m_listener;
	std::list< Connection > m_connections;
	Timer m_timer;

	/*! Receive available data, dispatch when request is complete; false when connection should be closed. */
	bool receive(Connection& connection, double now)
	{
		char buffer[4096];
		const int32_t nread = connection.socket->recv(buffer, sizeof(buffer));
		if (nread <= 0)
			return false;

		connection.received = now;

		if (!connection.request)
		{
			connection.header.append(buffer, nread);

			// Header ends at first blank line, CRLF or LF only.
			const size_t crlf = connection.header.find("\r\n\r\n");
			const size_t lf = connection.header.find("\n\n");
			size_t end = std::string::npos;
			if (crlf != std::string::npos && (lf == std::string::npos || crlf < lf))
				end = crlf + 4;
			else if (lf != std::string::npos)
				end = lf + 2;

			if (end == std::string::npos)
				return connection.header.size() <= c_maxHeaderSize;

			connection.body = connection.header.substr(end);
			connection.header.resize(end);

			connection.request = HttpRequest::parse(decodeHeader(connection.header));
			if (!connection.request)
				return false;

			const HttpRequest::Method method = connection.request->getMethod();
			if (method == HttpRequest::MtPost || method == HttpRequest::MtPut)
			{
				connection.contentLength = parseString< int32_t >(connection.request->getValue(L"Content-Length"));
				if (connection.contentLength > c_maxContentLength)
				{
					log::warning << L"Got PUT/POST request with too large content; connection dropped." << Endl;
					return false;
				}
			}
		}
		else
			connection.body.append(buffer, nread);

		if ((int64_t)connection.body.size() < connection.contentLength)
			return true;

		dispatch(connection);
		return false;
	}

	void dispatch(Connection& connection)
	{
		const HttpRequest* request = connection.request;

		// Extract session id from cookie.
		std::wstring session;
		for (const auto& kv : StringSplit< std::wstring >(request->getValue(L"Cookie"), L";"))
		{
			const size_t p = kv.find(L'=');
			if (p != kv.npos && trim(kv.substr(0, p)) == L"SESSIONID")
			{
				session = trim(kv.substr(p + 1));
				break;
			}
		}

		StringOutputStream ss;
		Ref< IStream > ds;
		std::map< std::wstring, std::wstring > headers;
		int32_t result = 503;

		if (m_listener)
		{
			const HttpRequest::Method method = request->getMethod();
			if (method == HttpRequest::MtPost || method == HttpRequest::MtPut)
			{
				if (connection.contentLength > 0)
				{
					Ref< MemoryStream > payloadStream = new MemoryStream(connection.body.data(), connection.contentLength);
					result = m_listener->httpClientRequest(m_server, request, payloadStream, ss, ds, headers, session);
				}
				else
					log::warning << L"Got PUT/POST request but no \"Content-Length\"; ignoring request." << Endl;
			}
			else
				result = m_listener->httpClientRequest(m_server, request, nullptr, ss, ds, headers, session);
		}

		SocketStream clientStream(connection.socket, false, true, c_writeTimeout);
		FileOutputStream os(&clientStream, new Utf8Encoding(), OutputStream::LineEnd::Win);
		if (result >= 200 && result < 300)
			os << L"HTTP/1.1 " << result << L" OK" << Endl;
		else
			os << L"HTTP/1.1 " << result << L" ERROR" << Endl;

		// Session cookie is hidden from scripts and not sent with cross-site posts.
		if (!session.empty())
			os << L"Set-Cookie: SESSIONID=" << session << L"; Path=/; HttpOnly; SameSite=Lax" << Endl;

		for (const auto& header : headers)
			os << header.first << L": " << header.second << Endl;

		os << L"Connection: close" << Endl;
		os << Endl;

		if (ds)
		{
			if (!StreamCopy(&clientStream, ds).execute())
				log::error << L"Unable to transfer entire stream to client; partially transmitted data." << Endl;
		}
		else
			os << ss.str();
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpServer", HttpServer, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpServer.IRequestListener", HttpServer::IRequestListener, Object)

bool HttpServer::create(const SocketAddressIPv4& bind)
{
	if (m_impl)
		return false;

	Ref< HttpServerImpl > impl = new HttpServerImpl(this);
	if (!impl->create(bind))
		return false;

	m_impl = impl;
	return true;
}

void HttpServer::destroy()
{
	safeDestroy(m_impl);
}

int32_t HttpServer::getListenPort()
{
	if (m_impl)
		return m_impl->getListenPort();
	else
		return 0;
}

void HttpServer::setRequestListener(IRequestListener* listener)
{
	if (m_impl)
		m_impl->setRequestListener(listener);
}

void HttpServer::update(int32_t duration)
{
	if (m_impl)
		m_impl->update(duration);
}

}
