/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/StringSplit.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Http/HttpRequest.h"
#include "Net/Http/HttpServer.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

class HttpServerImpl : public Object
{
public:
	HttpServerImpl(HttpServer* server)
	:	m_server(server)
	{
	}

	virtual ~HttpServerImpl()
	{
		destroy();
	}

	bool create(const SocketAddressIPv4& bind)
	{
		if (!m_serverSocket.bind(bind))
			return false;

		if (!m_serverSocket.listen())
			return false;

		return true;
	}

	void destroy()
	{
		m_listener = 0;
		m_serverSocket.close();
	}

	int32_t getListenPort()
	{
		return dynamic_type_cast< net::SocketAddressIPv4* >(m_serverSocket.getLocalAddress())->getPort();
	}

	void setRequestListener(HttpServer::IRequestListener* listener)
	{
		m_listener = listener;
	}

	void update(int32_t duration)
	{
		for (; duration >= 0; duration -= 100)
		{
			if (!m_serverSocket.select(true, false, false, 100))
				continue;

			Ref< TcpSocket > clientSocket = m_serverSocket.accept();
			if (!clientSocket)
				continue;

			if (!clientSocket->select(true, false, false, 10000))
			{
				clientSocket->close();
				clientSocket = 0;
				continue;
			}

			SocketStream clientStream(clientSocket, true, true, 100);
			StringReader clientReader(&clientStream, new Utf8Encoding());

			StringOutputStream ss;
			for (;;)
			{
				std::wstring tmp;
				if (clientReader.readLine(tmp) < 0)
					break;

				if (!tmp.empty())
					ss << tmp << Endl;
				else
				{
					if (clientReader.readLine(tmp) < 0)
						break;
					if (!tmp.empty())
						ss << tmp << Endl;
					else
						break;
				}
			}

			clientStream.setTimeout(10000);

			Ref< HttpRequest > request = HttpRequest::parse(ss.str());
			if (request)
			{
				StringOutputStream ssr;
				Ref< IStream > ds;
				int32_t result = 503;
				bool cache = true;
				std::wstring session;

				// Extract session id from cookie.
				if (request->hasValue(L"Cookie"))
				{
					std::wstring cookie = request->getValue(L"Cookie");
					
					StringSplit< std::wstring > ss(cookie, L";");
					for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
					{
						const std::wstring& kv = *i;
						
						size_t p = kv.find(L'=');
						if (p != kv.npos)
						{
							std::wstring k = kv.substr(0, p);
							if (k == L"SESSIONID")
							{
								session = kv.substr(p + 1);
								break;
							}
						}
					}
				}

				if (m_listener)
					result = m_listener->httpClientRequest(m_server, request, ssr, ds, cache, session);

				FileOutputStream os(&clientStream, new Utf8Encoding(), OutputStream::LeWin);
				if (result >= 200 && result < 300)
					os << L"HTTP/1.1 " << result << L" OK" << Endl;
				else
					os << L"HTTP/1.1 " << result << L" ERROR" << Endl;

				// Update cookie if necessary.
				if (!session.empty())
					os << L"Set-Cookie: SESSIONID=" << session << L";path=/" << Endl;

				if (!cache)
					os << L"Cache-Control: no-cache" << Endl;

				os << L"Connection: close" << Endl;
				os << Endl;

				if (ds)
				{
					if (!StreamCopy(&clientStream, ds).execute())
						log::error << L"Unable to transfer entire stream to client; partially transmitted data." << Endl;
				}
				else
					os << ssr.str();
			}

			safeClose(clientSocket);
		}
	}

private:
	HttpServer* m_server;
	TcpSocket m_serverSocket;
	Ref< HttpServer::IRequestListener > m_listener;
};

		}

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
}
