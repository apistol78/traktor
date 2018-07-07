/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpChunkStream.h"
#include "Net/Http/HttpResponse.h"
#include "Net/Http/IHttpRequestContent.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

bool executeRequest(const wchar_t* const method, const net::Url& url, const IHttpRequestContent* content, Ref< HttpResponse >& outResponse, Ref< IStream >& outStream)
{
	if (url.getProtocol() != L"http")
		return false;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return false;
	
	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
		return false;

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
	ss << method << L" " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";

	if (content)
	{
		std::wstring uec = content->getUrlEncodedContent();
		ss << L"Content-Type: application/x-www-form-urlencoded\r\n";
		ss << L"Content-Length: " << uec.length() << L"\r\n";
		ss << L"\r\n";
		ss << uec;
	}
	else
		ss << L"\r\n";

	outStream = new SocketStream(socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(outStream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	outResponse = new HttpResponse();
	if (!outResponse->parse(outStream))
		return false;

	// Reset offset in stream to origin.
	outStream->seek(IStream::SeekSet, 0);

	// Create chunked-transfer stream if such encoding is required.
	if (outResponse->get(L"Transfer-Encoding") == L"chunked")
		outStream = new HttpChunkStream(outStream);

	// If response contains content length field we can cap stream.
	int contentLength = parseString< int >(outResponse->get(L"Content-Length"));
	if (contentLength > 0)
		outStream = new StreamStream(outStream, contentLength);

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpClient", HttpClient, Object)

Ref< HttpResponse > HttpClient::get(const net::Url& url)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"GET", url, 0, response, m_stream))
		return response;
	else
		return 0;
}

Ref< HttpResponse > HttpClient::get(const net::Url& url, const IHttpRequestContent& content)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"GET", url, &content, response, m_stream))
		return response;
	else
		return 0;
}

Ref< HttpResponse > HttpClient::put(const net::Url& url)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"PUT", url, 0, response, m_stream))
		return response;
	else
		return 0;
}

Ref< HttpResponse > HttpClient::put(const net::Url& url, const IHttpRequestContent& content)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"PUT", url, &content, response, m_stream))
		return response;
	else
		return 0;
}

Ref< HttpResponse > HttpClient::post(const net::Url& url)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"POST", url, 0, response, m_stream))
		return response;
	else
		return 0;
}

Ref< HttpResponse > HttpClient::post(const net::Url& url, const IHttpRequestContent& content)
{
	Ref< HttpResponse > response;
	if (executeRequest(L"POST", url, &content, response, m_stream))
		return response;
	else
		return 0;
}

Ref< IStream > HttpClient::getStream()
{
	return m_stream;
}

	}
}
