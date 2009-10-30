#include <cstring>
#include "Net/SocketAddressIPv4.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketAddressIPv4", SocketAddressIPv4, SocketAddress)

SocketAddressIPv4::SocketAddressIPv4()
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = 0;
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const sockaddr_in& sockaddr)
:	m_sockaddr(sockaddr)
{
}

SocketAddressIPv4::SocketAddressIPv4(uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const uint8_t ip[4], uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = *reinterpret_cast< const uint32_t* >(ip);
}

SocketAddressIPv4::SocketAddressIPv4(const std::wstring& host, uint16_t port)
{
	// Try to resolve address, first try string denoted IP number as it will
	// probably fail faster than gethostbyname.
	uint32_t ia = inet_addr(wstombs(host).c_str());

#if !defined(_XBOX)
	if (ia == INADDR_NONE)
	{
		hostent* hostent = gethostbyname(wstombs(host).c_str());
		if (hostent != 0)
			ia = *reinterpret_cast< uint32_t* >(hostent->h_addr_list[0]);
	}
#endif

	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = ia;
}

bool SocketAddressIPv4::valid() const
{
	return bool(m_sockaddr.sin_port != 0);
}

std::wstring SocketAddressIPv4::getHostName() const
{
#if !defined(_XBOX)
	return mbstows(inet_ntoa(*const_cast< in_addr* >(&m_sockaddr.sin_addr)));
#else
	return L"<unsupported>";
#endif
}

uint16_t SocketAddressIPv4::getPort() const
{
	return ntohs(m_sockaddr.sin_port);
}

void SocketAddressIPv4::setSockAddr(const sockaddr_in& sockaddr)
{
	std::memcpy(&m_sockaddr, &sockaddr, sizeof(sockaddr_in));
}

const sockaddr_in& SocketAddressIPv4::getSockAddr() const
{
	return m_sockaddr;
}

	}
}
