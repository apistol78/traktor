/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class File;
class IStream;

	namespace net
	{

class SocketAddressIPv4;
class SocketAddressIPv6;
class TcpSocket;

class T_DLLCLASS FtpClient : public Object
{
	T_RTTI_CLASS;

public:
	bool connect(const SocketAddressIPv4& socketAddress);

	bool connect(const SocketAddressIPv6& socketAddress);

	void disconnect();

	bool login(const std::wstring& user, const std::wstring& pwd);

	void logout();

	void setCurrentDirectory(const std::wstring& cd);

	std::wstring getCurrentDirectory();

	bool getFileList(RefArray< File >& outFiles);

	Ref< IStream > open(const std::wstring& fileName);

private:
	Ref< TcpSocket > m_socket;
	Ref< IStream > m_commandStream;
};

	}
}

