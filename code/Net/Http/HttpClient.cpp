#include "Core/Io/StringOutputStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpResponse.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpClient", HttpClient, Object)

Ref< HttpResponse > HttpClient::get(const net::Url& url, const std::wstring& content)
{
	if (url.getProtocol() != L"http")
		return 0;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return 0;
	
	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
		return 0;

	// Build GET string.
	std::wstring resource = url.getPath();
	
	std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;
	
	std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Encode content.
	std::wstring urlec = Url::encode(content);

	// Create request header.
	StringOutputStream ss;
	ss << L"GET " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";
	ss << L"Content-Type: application/x-www-form-urlencoded\r\n";
	ss << L"Content-Length: " << urlec.length() << L"\r\n";
	ss << L"\r\n";
	ss << urlec;

	Ref< IStream > stream = new SocketStream(socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(stream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	Ref< HttpResponse > response = new HttpResponse();
	if (!response->parse(stream))
		return 0;

	return response;
}

Ref< HttpResponse > HttpClient::put(const net::Url& url, const std::wstring& content)
{
	if (url.getProtocol() != L"http")
		return 0;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return 0;
	
	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
		return 0;

	// Build PUT string.
	std::wstring resource = url.getPath();
	
	std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;
	
	std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Encode content.
	std::wstring urlec = Url::encode(content);

	// Create request header.
	StringOutputStream ss;
	ss << L"PUT " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";
	ss << L"Content-Type: application/x-www-form-urlencoded\r\n";
	ss << L"Content-Length: " << urlec.length() << L"\r\n";
	ss << L"\r\n";
	ss << urlec;

	Ref< IStream > stream = new SocketStream(socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(stream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	Ref< HttpResponse > response = new HttpResponse();
	if (!response->parse(stream))
		return 0;

	return response;
}

Ref< HttpResponse > HttpClient::post(const net::Url& url, const std::wstring& content)
{
	if (url.getProtocol() != L"http")
		return 0;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return 0;
	
	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
		return 0;

	// Build POST string.
	std::wstring resource = url.getPath();
	
	std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;
	
	std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Encode content.
	std::wstring urlec = Url::encode(content);

	// Create request header.
	StringOutputStream ss;
	ss << L"POST " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";
	ss << L"Content-Type: application/x-www-form-urlencoded\r\n";
	ss << L"Content-Length: " << urlec.length() << L"\r\n";
	ss << L"\r\n";
	ss << urlec;

	Ref< IStream > stream = new SocketStream(socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(stream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	Ref< HttpResponse > response = new HttpResponse();
	if (!response->parse(stream))
		return 0;

	return response;
}

	}
}
