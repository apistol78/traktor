/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
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

namespace traktor::net
{
	namespace
	{

void executeRequestJob(std::wstring method, net::Url url, Ref< const IHttpRequestContent > content, Ref< HttpClientResult > result)
{
	if (url.getProtocol() != L"http")
	{
		result->fail();
		return;
	}

	// Lookup host address.
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
	if (!addr.valid())
	{
		result->fail();
		return;
	}

	// Create and connect socket to host.
	Ref< TcpSocket > socket = new TcpSocket();
	if (!socket->connect(addr))
	{
		result->fail();
		return;
	}

	// Build GET string.
	std::wstring resource = url.getPath();

	const std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;

	const std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Create request header.
	DynamicMemoryStream ms(false, true);
	FileOutputStream ss(&ms, new Utf8Encoding());

	ss << method << L" " << resource << L" HTTP/1.1\r\n";
	ss << L"Host: " << url.getHost() << L"\r\n";
	ss << L"User-Agent: traktor/1.0\r\n";
	ss << L"Accept: */*\r\n";

	if (content)
	{
		ss << L"Content-Type: " << content->getContentType() << L"\r\n";
	 	ss << L"Content-Length: " << content->getContentLength() << L"\r\n";
	 	ss << L"\r\n";
		ss << L"\r\n";

		if (!content->encodeIntoStream(&ms))
		{
			result->fail();
			return;
		}		 
	}
	else
	{
		ss << L"\r\n";
		ss << L"\r\n";
	}

	// Send request and payload.
	Ref< IStream > stream = new SocketStream(socket);
	stream->write(ms.getBuffer().c_ptr(), ms.getBuffer().size());

	// Accept and parse response from server.
	Ref< HttpResponse > response = new HttpResponse();
	if (!response->parse(stream))
	{
		result->fail();
		return;
	}

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
	result->m_job = JobManager::getInstance().add([=](){
		executeRequestJob(L"GET", url, content, result);
	});
	if (result->m_job)
		return result;
	else
		return nullptr;
}

Ref< HttpClientResult > HttpClient::put(const net::Url& url, const IHttpRequestContent* content)
{
	Ref< HttpClientResult > result = new HttpClientResult();
	result->m_job = JobManager::getInstance().add([=](){
		executeRequestJob(L"PUT", url, content, result);
	});
	if (result->m_job)
		return result;
	else
		return nullptr;
}

Ref< HttpClientResult > HttpClient::post(const net::Url& url, const IHttpRequestContent* content)
{
	Ref< HttpClientResult > result = new HttpClientResult();
	result->m_job = JobManager::getInstance().add([=](){
		executeRequestJob(L"POST", url, content, result);
	});
	if (result->m_job)
		return result;
	else
		return nullptr;
}

}
