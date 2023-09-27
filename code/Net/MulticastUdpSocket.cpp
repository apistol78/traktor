/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Net/MulticastUdpSocket.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.MulticastUdpSocket", MulticastUdpSocket, UdpSocket)

bool MulticastUdpSocket::joinGroup(const SocketAddressIPv4& socketAddress)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	ip_mreq mr;
	mr.imr_multiaddr = socketAddress.getSockAddr().sin_addr;
	mr.imr_interface.s_addr = htonl(INADDR_ANY);

	return setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mr, sizeof(mr)) >= 0;
}

bool MulticastUdpSocket::joinGroup(const SocketAddressIPv6& socketAddress)
{
	return false;
}

bool MulticastUdpSocket::leaveGroup(const SocketAddressIPv4& socketAddress)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	ip_mreq mr;
	mr.imr_multiaddr = socketAddress.getSockAddr().sin_addr;
	mr.imr_interface.s_addr = htonl(INADDR_ANY);

	return setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char*)&mr, sizeof(mr)) >= 0;
}

bool MulticastUdpSocket::leaveGroup(const SocketAddressIPv6& socketAddress)
{
	return false;
}

bool MulticastUdpSocket::setLoopback(bool loopback)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	uint8_t lb = loopback ? 1 : 0;
	return setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&lb, sizeof(lb)) >= 0;
}

bool MulticastUdpSocket::setTTL(uint8_t ttl)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	return setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl)) >= 0;
}

}
