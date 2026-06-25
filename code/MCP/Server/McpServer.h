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
#include "Core/RefArray.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MCP_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class TcpSocket;

}

namespace traktor::mcp
{

class IMcpTool;
class IMcpPromptProvider;
class Json;

/*! Model Context Protocol server.
 * \ingroup MCP
 *
 * Hosts a JSON-RPC 2.0 endpoint over HTTP (bound to the loopback interface)
 * and dispatches the MCP core methods: "initialize", "ping", "tools/list"
 * and "tools/call". Domain capabilities are added by registering \a IMcpTool
 * implementations.
 *
 * The connection logic mirrors other Traktor servers (e.g. avalanche::Server):
 * \a create binds the listening socket, \a update accepts and services a single
 * pending connection and is expected to be driven from a dedicated thread, and
 * \a destroy releases the socket.
 */
class T_DLLCLASS McpServer : public Object
{
	T_RTTI_CLASS;

public:
	/*! MCP protocol revision advertised when a client does not request one. */
	constexpr static const wchar_t* c_protocolVersion = L"2025-06-18";

	/*! Bind the server to a loopback port and begin listening. */
	bool create(int32_t port);

	void destroy();

	/*! Accept and service a single pending connection.
	 *
	 * \return False if the server is no longer listening.
	 */
	bool update();

	/*! Register a tool; tools must be added before clients connect. */
	void addTool(IMcpTool* tool);

	/*! Register a prompt provider; providers must be added before clients connect. */
	void addPromptProvider(IMcpPromptProvider* provider);

	/*! Set the identity reported to clients in "initialize". */
	void setServerInfo(const std::wstring& name, const std::wstring& version);

	int32_t getListenPort() const;

	uint32_t getToolCount() const { return (uint32_t)m_tools.size(); }

	/*! Dispatch a single parsed JSON-RPC message (or batch).
	 *
	 * \param request Parsed request value; null indicates a parse error.
	 * \return Serialized response, or an empty string when no response is due
	 *         (i.e. the request was a notification).
	 */
	std::wstring dispatch(const Json* request);

private:
	Ref< net::TcpSocket > m_serverSocket;
	RefArray< IMcpTool > m_tools;
	RefArray< IMcpPromptProvider > m_promptProviders;
	std::wstring m_name = L"Traktor";
	std::wstring m_version = L"1.0.0";

	IMcpTool* findTool(const std::wstring& name) const;

	Ref< Json > handleMessage(const Json* message, bool& outResponse);

	Ref< Json > handleInitialize(const Json* params);

	Ref< Json > handleToolsList();

	Ref< Json > handleToolsCall(const Json* params, std::wstring& outError);

	Ref< Json > handlePromptsList();

	Ref< Json > handlePromptsGet(const Json* params, std::wstring& outError);

	Ref< Json > makeResult(Json* id, Json* result);

	Ref< Json > makeError(Json* id, int32_t code, const std::wstring& message);
};

}
