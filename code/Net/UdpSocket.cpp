/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/UdpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketAddressIPv6.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.UdpSocket", UdpSocket, Socket)

UdpSocket::UdpSocket()
:	Socket()
{
}

void UdpSocket::close()
{
	if (m_socket != INVALID_SOCKET)
	{
		CLOSE_SOCKET(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool UdpSocket::bind(const SocketAddressIPv4& socketAddress)
{
	struct sockaddr_in local = socketAddress.getSockAddr();
	
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(AF_INET, SOCK_DGRAM, 0);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	uint32_t opt = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
		return false;

#if defined(SO_REUSEPORT)
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEPORT, (const char*)&opt, sizeof(opt)) < 0)
		return false;
#endif

	if (::bind(m_socket, (sockaddr *)&local, sizeof(local)) < 0)
		return false;

	return true;
}

bool UdpSocket::bind(const SocketAddressIPv6& socketAddress)
{
#if !defined(__PS3__) && !defined(__PS4__) && !defined(EMSCRIPTEN) && !defined(_XBOX)
	const addrinfo* info = socketAddress.getAddrInfo(SOCK_DGRAM);
	if (!info)
		return false;

	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(info->ai_family, SOCK_DGRAM, info->ai_protocol);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	if (info->ai_family == AF_INET6)
	{
		int on = 1;
		::setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&on, sizeof(on));
	}

	uint32_t opt = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
		return false;

	if (::bind(m_socket, (sockaddr *)info->ai_addr, info->ai_addrlen) < 0)
		return false;

	return true;
#else
	return false;
#endif
}

bool UdpSocket::connect(const SocketAddressIPv4& socketAddress)
{
	struct sockaddr_in remote = socketAddress.getSockAddr();

	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(AF_INET, SOCK_DGRAM, 0);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	if (::connect(m_socket, (sockaddr *)&remote, sizeof(remote)) < 0)
		return false;

	return true;
}

bool UdpSocket::connect(const SocketAddressIPv6& socketAddress)
{
#if !defined(__PS3__) && !defined(__PS4__) && !defined(EMSCRIPTEN) && !defined(_XBOX)
	const addrinfo* info = socketAddress.getAddrInfo(SOCK_DGRAM);
	if (!info)
		return false;

	if (m_socket == INVALID_SOCKET)
	{
		m_socket = ::socket(info->ai_family, SOCK_DGRAM, info->ai_protocol);
		if (m_socket == INVALID_SOCKET)
			return false;
	}

	if (::connect(m_socket, (sockaddr *)info->ai_addr, info->ai_addrlen) < 0)
		return false;

	return true;
#else
	return false;
#endif
}

Ref< SocketAddress > UdpSocket::getLocalAddress()
{
	struct sockaddr_in in;

	socklen_t len = sizeof(in);
	if (getsockname(m_socket, (struct sockaddr*)&in, &len) != 0)
		return 0;

	return new SocketAddressIPv4(in);
}

int UdpSocket::sendTo(const SocketAddressIPv4& socketAddress, const void* data, int length)
{
	struct sockaddr_in to = socketAddress.getSockAddr();
	return int(::sendto(m_socket, static_cast<const char*>(data), length, 0, (sockaddr *)&to, sizeof(to)));
}

int UdpSocket::recvFrom(void* data, int length, SocketAddressIPv4* outSocketAddress)
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
