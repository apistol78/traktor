#include <cstring>
#include <sstream>
#include "Core/Misc/TString.h"
#include "Net/SocketAddressIPv6.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketAddressIPv6", SocketAddressIPv6, SocketAddress)

#if !defined(_PS3) && !defined(__EMSCRIPTEN__) && !defined(__PNACL__) && !defined(_XBOX)

SocketAddressIPv6::SocketAddressIPv6()
:	m_info(0)
{
}

SocketAddressIPv6::SocketAddressIPv6(uint16_t port)
:	m_info(0)
{
	addrinfo hints;

	std::stringstream ss; ss << port;
	std::string mbsp = ss.str();

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int ret = getaddrinfo(
		NULL,
		mbsp.c_str(),
		&hints,
		&m_info
	);
	if (ret != 0)
		m_info = 0;
}

SocketAddressIPv6::SocketAddressIPv6(const std::wstring& host, uint16_t port)
:	m_info(0)
{
	std::string mbsh = wstombs(host);
	addrinfo hints;

	std::stringstream ss; ss << port;
	std::string mbsp = ss.str();

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int ret = getaddrinfo(
		mbsh.c_str(),
		mbsp.c_str(),
		&hints,
		&m_info
	);
	if (ret != 0)
		m_info = 0;
}

SocketAddressIPv6::~SocketAddressIPv6()
{
	if (m_info)
	{
		freeaddrinfo(m_info);
		m_info = 0;
	}
}

bool SocketAddressIPv6::valid() const
{
	return bool(m_info != 0);
}

std::wstring SocketAddressIPv6::getHostName() const
{
	return L"<unsupported>";
}

const struct addrinfo* SocketAddressIPv6::getAddrInfo(int socktype) const
{
	addrinfo* iter;

	// Primarily use available IPv6 address.
	for (iter = m_info; iter; iter = iter->ai_next)
	{
		if (
			iter->ai_family == AF_INET6 &&
			iter->ai_socktype == socktype
		)
			return iter;
	}

	// No IPv6 address found, try falling back on IPv4.
	for (iter = m_info; iter; iter = iter->ai_next)
	{
		if (
			iter->ai_family == AF_INET &&
			iter->ai_socktype == socktype
		)
			return iter;
	}

	return 0;
}

#else	// _PS3

SocketAddressIPv6::SocketAddressIPv6()
{
}

SocketAddressIPv6::SocketAddressIPv6(uint16_t port)
{
}

SocketAddressIPv6::SocketAddressIPv6(const std::wstring& host, uint16_t port)
{
}

SocketAddressIPv6::~SocketAddressIPv6()
{
}

bool SocketAddressIPv6::valid() const
{
	return false;
}

std::wstring SocketAddressIPv6::getHostName() const
{
	return L"";
}

const struct addrinfo* SocketAddressIPv6::getAddrInfo(int socktype) const
{
	return 0;
}

#endif

	}
}
