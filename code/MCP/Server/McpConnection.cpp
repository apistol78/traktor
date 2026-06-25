/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/McpConnection.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/McpServer.h"
#include "Net/Http/HttpRequest.h"
#include "Net/TcpSocket.h"

#include <cstring>

namespace traktor::mcp
{
namespace
{

const size_t c_maxHeaderSize = 64 * 1024;
const int32_t c_readTimeout = 10000;

void encodeUtf8(const std::wstring& text, AlignedVector< uint8_t >& out)
{
	out.resize(0);
	if (text.empty())
		return;

	Utf8Encoding encoding;
	out.resize(text.size() * 4 + 4);
	const int n = encoding.translate(text.c_str(), (int32_t)text.size(), out.ptr());
	out.resize(n > 0 ? n : 0);
}

void append(AlignedVector< uint8_t >& buffer, const uint8_t* data, size_t size)
{
	const size_t at = buffer.size();
	buffer.resize(at + size);
	std::memcpy(buffer.ptr() + at, data, size);
}

/*! Find the start of the "\r\n\r\n" header terminator, or -1 if not present. */
int64_t findHeaderEnd(const AlignedVector< uint8_t >& buffer)
{
	if (buffer.size() < 4)
		return -1;
	const uint8_t* p = buffer.c_ptr();
	for (size_t i = 0; i + 3 < buffer.size(); ++i)
		if (p[i] == '\r' && p[i + 1] == '\n' && p[i + 2] == '\r' && p[i + 3] == '\n')
			return (int64_t)i;
	return -1;
}

std::wstring decodeAscii(const uint8_t* data, size_t size)
{
	std::wstring out;
	out.reserve(size);
	for (size_t i = 0; i < size; ++i)
		out.push_back((wchar_t)data[i]);
	return out;
}

/*! Read the "Content-Length" header value (case-insensitive). */
int32_t parseContentLength(const std::wstring& headers)
{
	const std::wstring lower = toLower(headers);
	const size_t key = lower.find(L"content-length:");
	if (key == std::wstring::npos)
		return 0;

	const size_t valueStart = key + 15;
	const size_t lineEnd = headers.find(L"\r\n", valueStart);
	const std::wstring value = (lineEnd == std::wstring::npos)
		? headers.substr(valueStart)
		: headers.substr(valueStart, lineEnd - valueStart);
	return parseString< int32_t >(trim(value), 0);
}

}

McpConnection::McpConnection(net::TcpSocket* clientSocket)
	: m_socket(clientSocket)
{
}

void McpConnection::process(McpServer* server)
{
	// Read raw request bytes ourselves rather than through a text reader; this
	// avoids any look-ahead stranding or consuming part of the request body.
	// We pull whatever is currently available with each read instead of trying
	// to fill a fixed-size buffer: an HTTP request has no length known up front,
	// so a fill-the-buffer read would block on the socket timeout once the client
	// has sent the whole request and is waiting for our response.
	AlignedVector< uint8_t > buffer;
	uint8_t chunk[1024];

	// Read until the end of the header block (CRLF CRLF).
	int64_t headerEnd = -1;
	for (;;)
	{
		const int64_t n = readSome(chunk, sizeof(chunk));
		if (n <= 0)
			break;

		append(buffer, chunk, (size_t)n);

		headerEnd = findHeaderEnd(buffer);
		if (headerEnd >= 0 || buffer.size() > c_maxHeaderSize)
			break;
	}
	if (headerEnd < 0)
		return;

	const std::wstring headerText = decodeAscii(buffer.c_ptr(), (size_t)headerEnd);

	Ref< net::HttpRequest > request = net::HttpRequest::parse(headerText);
	if (!request)
	{
		sendResponse(400, L"Bad Request", L"", L"");
		return;
	}

	const net::HttpRequest::Method method = request->getMethod();

	// CORS pre-flight.
	if (method == net::HttpRequest::MtOptions)
	{
		sendPreflight();
		return;
	}

	// MCP JSON-RPC messages are delivered via POST; we do not offer a server
	// initiated SSE stream on GET.
	if (method != net::HttpRequest::MtPost)
	{
		sendResponse(405, L"Method Not Allowed", L"", L"");
		return;
	}

	const int32_t contentLength = parseContentLength(headerText);

	// Bytes that follow the terminator are the start of the body; read more
	// from the socket until the whole body is buffered.
	const size_t bodyStart = (size_t)headerEnd + 4;
	while ((int64_t)(buffer.size() - bodyStart) < contentLength)
	{
		const int64_t n = readSome(chunk, sizeof(chunk));
		if (n <= 0)
			break;
		append(buffer, chunk, (size_t)n);
	}

	Ref< Json > jsonRequest;
	if (contentLength > 0 && (int64_t)(buffer.size() - bodyStart) >= contentLength)
	{
		MemoryStream bodyStream(buffer.ptr() + bodyStart, contentLength, true, false);
		jsonRequest = Json::parse(&bodyStream);
	}

	const std::wstring responseText = server->dispatch(jsonRequest);
	if (responseText.empty())
	{
		// Notification(s) only; acknowledge with no body.
		sendResponse(202, L"Accepted", L"", L"");
		return;
	}

	sendResponse(200, L"OK", L"application/json", responseText);
}

int64_t McpConnection::readSome(uint8_t* data, size_t size)
{
	// A single select + recv returns whatever bytes are currently buffered (up
	// to "size"), rather than blocking until "size" bytes have arrived. Returns
	// the byte count, 0 on graceful close, or -1 on timeout/error.
	if (m_socket->select(true, false, false, c_readTimeout) <= 0)
		return -1;
	return (int64_t)m_socket->recv(data, (int)size);
}

bool McpConnection::sendAll(const uint8_t* data, size_t size)
{
	size_t sent = 0;
	while (sent < size)
	{
		const int n = m_socket->send(data + sent, (int)(size - sent));
		if (n <= 0)
			return false;
		sent += (size_t)n;
	}
	return true;
}

void McpConnection::sendResponse(int32_t status, const std::wstring& statusText, const std::wstring& contentType, const std::wstring& body)
{
	AlignedVector< uint8_t > bodyBytes;
	encodeUtf8(body, bodyBytes);

	std::wstring header;
	header += L"HTTP/1.1 " + toString(status) + L" " + statusText + L"\r\n";
	if (!contentType.empty())
		header += L"Content-Type: " + contentType + L"\r\n";
	header += L"Access-Control-Allow-Origin: *\r\n";
	header += L"Content-Length: " + toString((int32_t)bodyBytes.size()) + L"\r\n";
	header += L"Connection: close\r\n";
	header += L"\r\n";

	AlignedVector< uint8_t > headerBytes;
	encodeUtf8(header, headerBytes);

	if (!sendAll(headerBytes.ptr(), headerBytes.size()))
		return;
	if (!bodyBytes.empty())
		sendAll(bodyBytes.ptr(), bodyBytes.size());
}

void McpConnection::sendPreflight()
{
	std::wstring header;
	header += L"HTTP/1.1 204 No Content\r\n";
	header += L"Access-Control-Allow-Origin: *\r\n";
	header += L"Access-Control-Allow-Methods: POST, OPTIONS\r\n";
	header += L"Access-Control-Allow-Headers: Content-Type, Mcp-Session-Id, Mcp-Protocol-Version, Authorization\r\n";
	header += L"Access-Control-Max-Age: 86400\r\n";
	header += L"Content-Length: 0\r\n";
	header += L"Connection: close\r\n";
	header += L"\r\n";

	AlignedVector< uint8_t > headerBytes;
	encodeUtf8(header, headerBytes);
	sendAll(headerBytes.ptr(), headerBytes.size());
}

}
