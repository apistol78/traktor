/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if !defined(_WIN32)
#	include <csignal>
#endif
#include "MCP/Server/McpServer.h"

#include "Core/Log/Log.h"
#include "MCP/Server/IMcpPromptProvider.h"
#include "MCP/Server/IMcpTool.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/McpConnection.h"
#include "Net/SocketAddress.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor::mcp
{
namespace
{

// JSON-RPC 2.0 error codes.
const int32_t c_errorParse = -32700;
const int32_t c_errorInvalidRequest = -32600;
const int32_t c_errorMethodNotFound = -32601;
const int32_t c_errorInvalidParams = -32602;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.McpServer", McpServer, Object)

bool McpServer::create(int32_t port)
{
	if (m_serverSocket)
		return false;

#if !defined(_WIN32)
	// Writing to a socket whose peer has already closed the connection raises
	// SIGPIPE, whose default action terminates the host process. Ignore it so a
	// failed send simply returns an error (handled in McpConnection::sendAll).
	signal(SIGPIPE, SIG_IGN);
#endif

	// Bind to the loopback interface only; the MCP endpoint should never be
	// reachable from outside the local machine.
	Ref< net::TcpSocket > serverSocket = new net::TcpSocket();
	if (!serverSocket->bind(net::SocketAddressIPv4(L"127.0.0.1", (uint16_t)port), true))
	{
		log::error << L"MCP server; unable to bind to 127.0.0.1:" << port << L"." << Endl;
		return false;
	}
	if (!serverSocket->listen())
	{
		log::error << L"MCP server; unable to listen on 127.0.0.1:" << port << L"." << Endl;
		return false;
	}

	m_serverSocket = serverSocket;
	return true;
}

void McpServer::destroy()
{
	if (m_serverSocket)
	{
		m_serverSocket->close();
		m_serverSocket = nullptr;
	}
	m_tools.clear();
	m_promptProviders.clear();
}

bool McpServer::update()
{
	if (!m_serverSocket)
		return false;

	// Wait briefly for an incoming connection; keep the timeout short so the
	// driving thread can observe a stop request promptly.
	if (m_serverSocket->select(true, false, false, 100) <= 0)
		return true;

	Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
	if (!clientSocket)
		return true;

	clientSocket->setNoDelay(true);

	McpConnection connection(clientSocket);
	connection.process(this);
	return true;
}

void McpServer::addTool(IMcpTool* tool)
{
	if (tool)
		m_tools.push_back(tool);
}

void McpServer::addPromptProvider(IMcpPromptProvider* provider)
{
	if (provider)
		m_promptProviders.push_back(provider);
}

void McpServer::setServerInfo(const std::wstring& name, const std::wstring& version)
{
	m_name = name;
	m_version = version;
}

int32_t McpServer::getListenPort() const
{
	if (!m_serverSocket)
		return 0;

	Ref< net::SocketAddress > localAddress = m_serverSocket->getLocalAddress();
	if (auto address = dynamic_type_cast< net::SocketAddressIPv4* >(localAddress))
		return address->getPort();

	return 0;
}

std::wstring McpServer::dispatch(const Json* request)
{
	if (!request)
		return makeError(nullptr, c_errorParse, L"Parse error")->toString();

	// JSON-RPC batch.
	if (request->isArray())
	{
		Ref< Json > responses = Json::createArray();
		for (uint32_t i = 0; i < request->size(); ++i)
		{
			bool response = false;
			Ref< Json > result = handleMessage(request->at(i), response);
			if (response && result)
				responses->push(result);
		}
		if (responses->size() == 0)
			return L"";
		return responses->toString();
	}

	// Single message.
	bool response = false;
	Ref< Json > result = handleMessage(request, response);
	if (!response || !result)
		return L"";
	return result->toString();
}

IMcpTool* McpServer::findTool(const std::wstring& name) const
{
	for (auto tool : m_tools)
		if (tool->getName() == name)
			return tool;
	return nullptr;
}

Ref< Json > McpServer::handleMessage(const Json* message, bool& outResponse)
{
	outResponse = false;

	if (!message || !message->isObject())
	{
		outResponse = true;
		return makeError(nullptr, c_errorInvalidRequest, L"Invalid request");
	}

	Json* method = message->getMember(L"method");
	Json* id = message->getMember(L"id");

	// A request without an "id" is a notification and must not be answered.
	const bool notification = (id == nullptr);

	if (!method || !method->isString())
	{
		if (notification)
			return nullptr;
		outResponse = true;
		return makeError(id, c_errorInvalidRequest, L"Invalid request");
	}

	const std::wstring methodName = method->getString();

	if (notification)
	{
		// Known notifications (e.g. "notifications/initialized") are accepted
		// silently; unknown notifications are ignored per JSON-RPC.
		return nullptr;
	}

	outResponse = true;

	if (methodName == L"initialize")
		return makeResult(id, handleInitialize(message->getMember(L"params")));
	else if (methodName == L"ping")
		return makeResult(id, Json::createObject());
	else if (methodName == L"tools/list")
		return makeResult(id, handleToolsList());
	else if (methodName == L"tools/call")
	{
		std::wstring error;
		Ref< Json > result = handleToolsCall(message->getMember(L"params"), error);
		if (!error.empty())
			return makeError(id, c_errorInvalidParams, error);
		return makeResult(id, result);
	}
	else if (methodName == L"prompts/list")
		return makeResult(id, handlePromptsList());
	else if (methodName == L"prompts/get")
	{
		std::wstring error;
		Ref< Json > result = handlePromptsGet(message->getMember(L"params"), error);
		if (!error.empty())
			return makeError(id, c_errorInvalidParams, error);
		return makeResult(id, result);
	}

	return makeError(id, c_errorMethodNotFound, L"Method not found: " + methodName);
}

Ref< Json > McpServer::handleInitialize(const Json* params)
{
	std::wstring protocolVersion;
	if (params && params->getMember(L"protocolVersion"))
		protocolVersion = params->getMember(L"protocolVersion")->getString();
	if (protocolVersion.empty())
		protocolVersion = c_protocolVersion;

	Ref< Json > toolsCapability = Json::createObject();
	toolsCapability->setBoolean(L"listChanged", false);

	Ref< Json > promptsCapability = Json::createObject();
	promptsCapability->setBoolean(L"listChanged", false);

	Ref< Json > capabilities = Json::createObject();
	capabilities->set(L"tools", toolsCapability);
	capabilities->set(L"prompts", promptsCapability);

	Ref< Json > serverInfo = Json::createObject();
	serverInfo->setString(L"name", m_name);
	serverInfo->setString(L"version", m_version);

	Ref< Json > result = Json::createObject();
	result->setString(L"protocolVersion", protocolVersion);
	result->set(L"capabilities", capabilities);
	result->set(L"serverInfo", serverInfo);
	result->setString(L"instructions", L"Traktor engine MCP server. Use tools/list to discover tools for inspecting the asset database.");
	return result;
}

Ref< Json > McpServer::handleToolsList()
{
	Ref< Json > tools = Json::createArray();
	for (auto tool : m_tools)
	{
		Ref< Json > entry = Json::createObject();
		entry->setString(L"name", tool->getName());
		entry->setString(L"description", tool->getDescription());

		Ref< Json > inputSchema = tool->getInputSchema();
		entry->set(L"inputSchema", inputSchema ? inputSchema : Json::createObject());

		tools->push(entry);
	}

	Ref< Json > result = Json::createObject();
	result->set(L"tools", tools);
	return result;
}

Ref< Json > McpServer::handlePromptsList()
{
	Ref< Json > prompts = Json::createArray();
	for (auto provider : m_promptProviders)
		provider->listPrompts(prompts);

	Ref< Json > result = Json::createObject();
	result->set(L"prompts", prompts);
	return result;
}

Ref< Json > McpServer::handlePromptsGet(const Json* params, std::wstring& outError)
{
	if (!params || !params->isObject())
	{
		outError = L"Invalid params";
		return nullptr;
	}

	Json* name = params->getMember(L"name");
	if (!name || !name->isString() || name->getString().empty())
	{
		outError = L"Missing prompt name";
		return nullptr;
	}

	const Json* arguments = params->getMember(L"arguments");
	for (auto provider : m_promptProviders)
	{
		std::wstring error;
		Ref< Json > result = provider->getPrompt(name->getString(), arguments, error);
		if (result)
			return result;
		if (!error.empty())
		{
			outError = error;
			return nullptr;
		}
	}

	outError = L"Unknown prompt: " + name->getString();
	return nullptr;
}

Ref< Json > McpServer::handleToolsCall(const Json* params, std::wstring& outError)
{
	if (!params || !params->isObject())
	{
		outError = L"Invalid params";
		return nullptr;
	}

	Json* name = params->getMember(L"name");
	if (!name || !name->isString() || name->getString().empty())
	{
		outError = L"Missing tool name";
		return nullptr;
	}

	IMcpTool* tool = findTool(name->getString());
	if (!tool)
	{
		outError = L"Unknown tool: " + name->getString();
		return nullptr;
	}

	const Json* arguments = params->getMember(L"arguments");

	std::wstring toolError;
	Ref< Json > value = tool->invoke(arguments, toolError);

	// Build a tool result. Execution failures are reported as a result with
	// "isError" set (so the model can read them), not as a protocol error.
	Ref< Json > content = Json::createArray();
	Ref< Json > result = Json::createObject();

	if (!toolError.empty())
	{
		Ref< Json > block = Json::createObject();
		block->setString(L"type", L"text");
		block->setString(L"text", toolError);
		content->push(block);
		result->set(L"content", content);
		result->setBoolean(L"isError", true);
		return result;
	}

	// A tool may return a pre-built MCP content array (e.g. a mix of text and
	// image blocks); pass it through verbatim. Object and string results take
	// the default text wrapping below.
	if (value && value->isArray())
	{
		result->set(L"content", value);
		result->setBoolean(L"isError", false);
		return result;
	}

	const std::wstring text = (value && value->isString()) ? value->getString() : (value ? value->toString() : L"");
	Ref< Json > block = Json::createObject();
	block->setString(L"type", L"text");
	block->setString(L"text", text);
	content->push(block);
	result->set(L"content", content);
	result->setBoolean(L"isError", false);

	if (value && value->isObject())
		result->set(L"structuredContent", value);

	return result;
}

Ref< Json > McpServer::makeResult(Json* id, Json* result)
{
	Ref< Json > response = Json::createObject();
	response->setString(L"jsonrpc", L"2.0");
	if (id)
		response->set(L"id", id);
	else
		response->set(L"id", Json::createNull());
	if (result)
		response->set(L"result", result);
	else
		response->set(L"result", Json::createObject());
	return response;
}

Ref< Json > McpServer::makeError(Json* id, int32_t code, const std::wstring& message)
{
	Ref< Json > error = Json::createObject();
	error->setNumber(L"code", code);
	error->setString(L"message", message);

	Ref< Json > response = Json::createObject();
	response->setString(L"jsonrpc", L"2.0");
	if (id)
		response->set(L"id", id);
	else
		response->set(L"id", Json::createNull());
	response->set(L"error", error);
	return response;
}

}
