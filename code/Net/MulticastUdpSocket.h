/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Net/UdpSocket.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! Multicast datagram socket.
 * \ingroup Net
 */
class T_DLLCLASS MulticastUdpSocket : public UdpSocket
{
	T_RTTI_CLASS;

public:
	bool joinGroup(const SocketAddressIPv4& socketAddress);

	bool joinGroup(const SocketAddressIPv6& socketAddress);

	bool leaveGroup(const SocketAddressIPv4& socketAddress);

	bool leaveGroup(const SocketAddressIPv6& socketAddress);

	bool setLoopback(bool loopback);

	bool setTTL(uint8_t ttl);
};

	}
}

