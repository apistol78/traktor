/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <sstream>
#include "Core/Misc/TString.h"
#include "Net/SocketAddressIPv6.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketAddressIPv6", SocketAddressIPv6, SocketAddress)

SocketAddressIPv6::SocketAddressIPv6(uint16_t port)
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
		m_info = nullptr;
}

SocketAddressIPv6::SocketAddressIPv6(const std::wstring& host, uint16_t port)
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
		m_info = nullptr;
}

SocketAddressIPv6::~SocketAddressIPv6()
{
	if (m_info)
	{
		freeaddrinfo(m_info);
		m_info = nullptr;
	}
}

bool SocketAddressIPv6::valid() const
{
	return bool(m_info != nullptr);
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

	return nullptr;
}

}
