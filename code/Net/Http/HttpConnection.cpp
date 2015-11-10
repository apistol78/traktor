#include "Net/Http/HttpConnection.h"
#include "Net/Http/HttpResponse.h"
#include "Net/Http/HttpChunkStream.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/SocketStream.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpConnection", HttpConnection, UrlConnection)

UrlConnection::EstablishResult HttpConnection::establish(const Url& url, Url* outRedirectionUrl)
{
	if (url.getProtocol() != L"http")
		return ErInvalidUrl;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return ErInvalidUrl;

	// Create and connect socket to host.
	m_socket = new TcpSocket();
	if (!m_socket->connect(addr))
		return ErConnectFailed;

	// Build GET string.
	std::wstring resource = url.getPath();
	
	std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;
	
	std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Create request header.
	StringOutputStream ss;
	ss << L"GET " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";
	ss << L"X-Requested-With: XMLHttpRequest\r\n";
	ss << L"\r\n\r\n";

	Ref< IStream > stream = new SocketStream(m_socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(stream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	HttpResponse response;
	if (!response.parse(stream))
	{
		log::error << L"Invalid HTTP response header" << Endl;
		return ErFailed;
	}
		
	// Ensure it's a positive response.
	if (response.getStatusCode() < 200 || response.getStatusCode() >= 300)
	{
		// Handle redirect response.
		if (response.getStatusCode() == 302 && outRedirectionUrl)
		{
			std::wstring location = response.get(L"Location");
			if (!location.empty())
			{
				*outRedirectionUrl = location;
				return ErRedirect;
			}
		}

		// Unhandled errornous response.
		log::error << L"HTTP error, " << response.getStatusCode() << L", " << response.getStatusMessage() << Endl;
		return ErFailed;
	}
	
	// Reset offset in stream to origin.
	stream->seek(IStream::SeekSet, 0);

	// Create chunked-transfer stream if such encoding is required.
	if (response.get(L"Transfer-Encoding") == L"chunked")
		stream = new HttpChunkStream(stream);

	// If response contains content length field we can cap stream.
	int contentLength = parseString< int >(response.get(L"Content-Length"));
	if (contentLength > 0)
		stream = new StreamStream(stream, contentLength);

	m_stream = stream;
	m_url = url;

	return ErSucceeded;
}

Url HttpConnection::getUrl() const
{
	return m_url;
}

Ref< IStream > HttpConnection::getStream()
{
	return m_stream;
}

	}
}
