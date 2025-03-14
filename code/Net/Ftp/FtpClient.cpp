/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Net/TcpSocket.h"
#include "Net/SocketStream.h"
#include "Net/Ftp/FtpClient.h"

namespace traktor::net
{
	namespace
	{

bool isError(uint32_t code)
{
	return code >= 400;
}

bool isFatal(uint32_t code)
{
	return code >= 500;
}

bool isPreliminary(uint32_t code)
{
	return code <= 199;
}

bool isIntermediate(uint32_t code)
{
	return code >= 400 && code <= 499;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.FtpClient", FtpClient, Object)

bool FtpClient::connect(const SocketAddressIPv4& socketAddress)
{
	m_socket = new TcpSocket();
	if (!m_socket->connect(socketAddress))
		return false;

	m_commandStream = new SocketStream(m_socket);
	return true;
}

bool FtpClient::connect(const SocketAddressIPv6& socketAddress)
{
	m_socket = new TcpSocket();
	if (!m_socket->connect(socketAddress))
		return false;

	m_commandStream = new SocketStream(m_socket);
	return true;
}

void FtpClient::disconnect()
{
	safeClose(m_commandStream);
	safeClose(m_socket);
}

bool FtpClient::login(const std::wstring& user, const std::wstring& pwd)
{
	return false;
}

void FtpClient::logout()
{
}

void FtpClient::setCurrentDirectory(const std::wstring& cd)
{
}

std::wstring FtpClient::getCurrentDirectory()
{
	return L"";
}

bool FtpClient::getFileList(RefArray< File >& outFiles)
{
	return false;
}

Ref< IStream > FtpClient::open(const std::wstring& fileName)
{
	return nullptr;
}

}
