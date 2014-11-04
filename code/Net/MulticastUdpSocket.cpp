#include "Net/MulticastUdpSocket.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor
{
	namespace net
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
#if !defined(_XBOX)
	if (m_socket == INVALID_SOCKET)
		return false;

	uint8_t lb = loopback ? 1 : 0;
	return setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&lb, sizeof(lb)) >= 0;
#else
	return false;
#endif
}

bool MulticastUdpSocket::setTTL(uint8_t ttl)
{
#if !defined(_XBOX)
	if (m_socket == INVALID_SOCKET)
		return false;

	return setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl)) >= 0;
#else
	return false;
#endif
}

	}
}
