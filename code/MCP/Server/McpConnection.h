/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstdint>
#include <string>
#include "Core/Ref.h"

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
 */
class McpConnection
{
public:
	explicit McpConnection(net::TcpSocket* clientSocket);

	void process(McpServer* server);

private:
	Ref< net::TcpSocket > m_socket;

	int64_t readSome(uint8_t* data, size_t size);

	bool sendAll(const uint8_t* data, size_t size);

	void sendResponse(int32_t status, const std::wstring& statusText, const std::wstring& contentType, const std::wstring& body);

	void sendPreflight();
};

}
