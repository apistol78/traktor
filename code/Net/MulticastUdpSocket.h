/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_MulticastUdpSocket_H
#define traktor_net_MulticastUdpSocket_H

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

/*! \brief Multicast datagram socket.
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

#endif	// traktor_net_MulticastUdpSocket_H
