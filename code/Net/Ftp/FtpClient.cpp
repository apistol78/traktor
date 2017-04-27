/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/Ftp/FtpClient.h"
#include "Net/TcpSocket.h"
#include "Net/SocketStream.h"

namespace traktor
{
	namespace net
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
	if (m_commandStream)
	{
		m_commandStream->close();
		m_commandStream = 0;
	}
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
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
	return 0;
}

	}
}
