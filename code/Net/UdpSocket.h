/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Net/Socket.h"

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

class SocketAddress;
class SocketAddressIPv4;
class SocketAddressIPv6;

/*! UDP datagram socket.
 * \ingroup Net
 */
class T_DLLCLASS UdpSocket : public Socket
{
	T_RTTI_CLASS;

public:
	UdpSocket() = default;

	virtual void close() override final;

	/*! Bind to local address. */
	bool bind(const SocketAddressIPv4& socketAddress);

	/*! Bind to local address. */
	bool bind(const SocketAddressIPv6& socketAddress);

	/*! Connect to remote host. */
	bool connect(const SocketAddressIPv4& socketAddress);

	/*! Connect to remote host. */
	bool connect(const SocketAddressIPv6& socketAddress);

	/*! Get local socket address. */
	Ref< SocketAddress > getLocalAddress();

	/*! Send data directly to address. */
	int sendTo(const SocketAddressIPv4& socketAddress, const void* data, int length);

	/*! \breif Receive block of data. */
	int recvFrom(void* data, int length, SocketAddressIPv4* outSocketAddress);
};

	}
}

