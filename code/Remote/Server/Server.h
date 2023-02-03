/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_REMOTE_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class DiscoveryManager;
class TcpSocket;

}

namespace traktor::remote
{

class T_DLLCLASS Server : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const std::wstring& scratchPath, const std::wstring& keyword, int32_t listenPort, bool verbose);

	void destroy();

	bool update();

	int32_t getListenPort() const { return m_listenPort; }

	const std::wstring& getScratchPath() const { return m_scratchPath; }

private:
	Ref< net::TcpSocket > m_serverSocket;
	Ref< net::DiscoveryManager > m_discoveryManager;
	int32_t m_listenPort = 0;
	std::wstring m_hostName;
	std::wstring m_scratchPath;
	std::wstring m_keyword;
	std::map< std::wstring, uint32_t > m_fileHashes;
	bool m_verbose = false;

	uint8_t handleDeploy(net::TcpSocket* clientSocket);

	uint8_t handleLaunchProcess(net::TcpSocket* clientSocket);

	uint8_t handleFetch(net::TcpSocket* clientSocket);

	void processClient(net::TcpSocket* clientSocket);
};

}
