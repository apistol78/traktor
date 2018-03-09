/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/IcmpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketAddressIPv6.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.IcmpSocket", IcmpSocket, Socket)

IcmpSocket::IcmpSocket()
:	Socket()
{
}

void IcmpSocket::close()
{
	if (m_socket != INVALID_SOCKET)
	{
		CLOSE_SOCKET(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool IcmpSocket::bind(const SocketAddressIPv4& socketAddress)
{
	struct sockaddr_in local = socketAddress.getSockAddr();
	
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	if (::bind(m_socket, (sockaddr *)&local, sizeof(local)) < 0)
		return false;

	return true;
}

bool IcmpSocket::bind(const SocketAddressIPv6& socketAddress)
{
	return false;
}

bool IcmpSocket::connect(const SocketAddressIPv4& socketAddress)
{
	struct sockaddr_in remote = socketAddress.getSockAddr();

	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	if (::connect(m_socket, (sockaddr *)&remote, sizeof(remote)) < 0)
		return false;

	return true;
}

bool IcmpSocket::connect(const SocketAddressIPv6& socketAddress)
{
	return false;
}

Ref< SocketAddress > IcmpSocket::getLocalAddress()
{
	struct sockaddr_in in;

	socklen_t len = sizeof(in);
	if (getsockname(m_socket, (struct sockaddr*)&in, &len) != 0)
		return 0;

	return new SocketAddressIPv4(in);
}

int IcmpSocket::sendTo(const SocketAddressIPv4& socketAddress, const void* data, int length)
{
	struct sockaddr_in to = socketAddress.getSockAddr();
	return int(::sendto(m_socket, static_cast<const char*>(data), length, 0, (sockaddr *)&to, sizeof(to)));
}

int IcmpSocket::recvFrom(void* data, int length, SocketAddressIPv4* outSocketAddress)
{
	struct sockaddr_in from;

	socklen_t fromlen = sizeof(from);
	int nrecv = int(::recvfrom(m_socket, static_cast<char*>(data), length, 0, (sockaddr *)&from, &fromlen));
	if (nrecv <= 0)
		return 0;

	if (outSocketAddress)
		outSocketAddress->setSockAddr(from);

	return nrecv;
}

	}
}
