/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Functor/Functor.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Core/Thread/JobManager.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Net/Http/HttpChunkStream.h"
#include "Net/Http/HttpResponse.h"
#include "Net/Http/IHttpRequestContent.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

void executeRequestJob(std::wstring method, net::Url url, Ref< const IHttpRequestContent > content, Ref< HttpClientResult > result)
{
	if (url.getProtocol() != L"http")
		return;
	
	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
		return;
	
	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
		return;

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

	Ref< IStream > stream = new SocketStream(socket);

	// Send request, UTF-8 encoded.
	FileOutputStream fos(stream, new Utf8Encoding());
	fos << ss.str();

	// Accept and parse response from server.
	Ref< HttpResponse > response = new HttpResponse();
	if (!response->parse(stream))
		return;

	// Reset offset in stream to origin.
	stream->seek(IStream::SeekSet, 0);

	// Create chunked-transfer stream if such encoding is required.
	if (response->get(L"Transfer-Encoding") == L"chunked")
		stream = new HttpChunkStream(stream);

	// If response contains content length field we can cap stream.
	int contentLength = parseString< int >(response->get(L"Content-Length"));
	if (contentLength > 0)
		stream = new StreamStream(stream, contentLength);

	result->succeed(response, stream);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpClient", HttpClient, Object)

Ref< HttpClientResult > HttpClient::get(const net::Url& url, const IHttpRequestContent* content)
{
	Ref< HttpClientResult > result = new HttpClientResult();
	result->m_job = JobManager::getInstance().add(makeStaticFunctor< std::wstring, net::Url, Ref< const IHttpRequestContent >, Ref< HttpClientResult > >(
		&executeRequestJob, L"GET", url, content, result)
	);
	if (result->m_job)
		return result;
	else
		return 0;
}

Ref< HttpClientResult > HttpClient::put(const net::Url& url, const IHttpRequestContent* content)
{
	Ref< HttpClientResult > result = new HttpClientResult();
	result->m_job = JobManager::getInstance().add(makeStaticFunctor< std::wstring, net::Url, Ref< const IHttpRequestContent >, Ref< HttpClientResult > >(
		&executeRequestJob, L"PUT", url, content, result)
	);
	if (result->m_job)
		return result;
	else
		return 0;
}

Ref< HttpClientResult > HttpClient::post(const net::Url& url, const IHttpRequestContent* content)
{
	Ref< HttpClientResult > result = new HttpClientResult();
	result->m_job = JobManager::getInstance().add(makeStaticFunctor< std::wstring, net::Url, Ref< const IHttpRequestContent >, Ref< HttpClientResult > >(
		&executeRequestJob, L"POST", url, content, result)
	);
	if (result->m_job)
		return result;
	else
		return 0;
}

	}
}
