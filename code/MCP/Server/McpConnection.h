/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

#include <atomic>
#include <cstdint>
#include <string>

namespace traktor
{

class Thread;

}

namespace traktor::net
{

class TcpSocket;

}

namespace traktor::mcp
{

class McpServer;

/*! Single MCP client connection.
 * \ingroup MCP
 *
 * Reads one HTTP request from the client socket, forwards the JSON-RPC body to
 * the server for dispatch, and writes back an HTTP response. The connection is
 * stateless and serviced once per accepted socket ("Connection: close").
 *
 * Each connection runs on its own pooled worker thread so that a slow request
 * (e.g. a tool that builds an asset or waits on the UI thread) never blocks the
 * server's accept loop or other in-flight connections. The owning McpServer
 * tracks live connections and reaps them once \a update reports completion.
 */
class McpConnection : public Object
{
	T_RTTI_CLASS;

public:
	explicit McpConnection(net::TcpSocket* clientSocket);

	virtual ~McpConnection();

	/*! Begin servicing the connection on a pooled worker thread. */
	bool create(McpServer* server);

	/*! \return False once the request has been fully serviced. */
	bool update() const;

private:
	Ref< net::TcpSocket > m_socket;
	Thread* m_thread = nullptr;
	std::atomic< bool > m_finished;

	void process(McpServer* server);

	int64_t readSome(uint8_t* data, size_t size);

	bool sendAll(const uint8_t* data, size_t size);

	void sendResponse(int32_t status, const std::wstring& statusText, const std::wstring& contentType, const std::wstring& body);

	void sendPreflight();
};

}
