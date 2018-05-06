/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_SocketAddressIPv4_H
#define traktor_net_SocketAddressIPv4_H

#include <list>
#include <string>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Net/Platform.h"
#include "Net/SocketAddress.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! \brief IPv4 socket address.
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

	SocketAddressIPv4(const sockaddr_in& sockaddr);

	SocketAddressIPv4(uint16_t port);

	SocketAddressIPv4(const uint32_t addr, uint16_t port);

	SocketAddressIPv4(const uint8_t add[4], uint16_t port);

	SocketAddressIPv4(const std::wstring& host, uint16_t port);

	virtual bool valid() const T_OVERRIDE T_FINAL;

	virtual std::wstring getHostName() const T_OVERRIDE T_FINAL;

	uint32_t getAddr() const;

	uint16_t getPort() const;

	void setSockAddr(const sockaddr_in& sockaddr);

	const sockaddr_in& getSockAddr() const;

	static bool getInterfaces(std::list< Interface >& outInterfaces);

	static bool getBestInterface(Interface& outInterface);

private:
	sockaddr_in m_sockaddr;
};

	}
}

#endif	// traktor_net_SocketAddressIPv4_H
