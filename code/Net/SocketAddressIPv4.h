/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <string>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Net/Platform.h"
#include "Net/SocketAddress.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

/*! IPv4 socket address.
 * \ingroup Net
 */
class T_DLLCLASS SocketAddressIPv4 : public SocketAddress
{
	T_RTTI_CLASS;

public:
	enum InterfaceType
	{
		ItDefault = 0,
		ItWiFi = 1,
		ItVPN = 2,
		ItLoopback = 3
	};

	struct Interface
	{
		InterfaceType type;
		Ref< SocketAddressIPv4 > addr;
	};

	SocketAddressIPv4();

	explicit SocketAddressIPv4(const sockaddr_in& sockaddr);

	explicit SocketAddressIPv4(uint16_t port);

	explicit SocketAddressIPv4(const uint32_t addr, uint16_t port);

	explicit SocketAddressIPv4(const uint8_t add[4], uint16_t port);

	explicit SocketAddressIPv4(const std::wstring& host, uint16_t port);

	virtual bool valid() const override final;

	virtual std::wstring getHostName() const override final;

	uint32_t getAddr() const;

	uint16_t getPort() const;

	void setSockAddr(const sockaddr_in& sockaddr);

	const sockaddr_in& getSockAddr() const;

	static bool getInterfaces(std::list< Interface >& outInterfaces);

	static bool getBestInterface(Interface& outInterface);

	bool operator == (const SocketAddressIPv4& rh) const;

private:
	sockaddr_in m_sockaddr;
};

}
