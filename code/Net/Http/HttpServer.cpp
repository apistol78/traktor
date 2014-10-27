#include "Core/RefArray.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
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

			Ref< HttpRequest > request = HttpRequest::parse(ss.str());
			if (request)
			{
				StringOutputStream ss;
				Ref< IStream > ds;
				int32_t result = 503;

				if (m_listener)
					result = m_listener->httpClientRequest(m_server, request, ss, ds);

				FileOutputStream os(&clientStream, new Utf8Encoding(), OutputStream::LeWin);
				if (result >= 200 && result < 300)
					os << L"HTTP/1.1 " << result << L" OK" << Endl;
				else
					os << L"HTTP/1.1 " << result << L" ERROR" << Endl;

				os << L"Connection: close" << Endl;
				os << Endl;

				if (ds)
					StreamCopy(&clientStream, ds).execute();
				else
					os << ss.str();
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
