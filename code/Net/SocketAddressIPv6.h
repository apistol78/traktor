/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_SocketAddressIPv6_H
#define traktor_net_SocketAddressIPv6_H

#include <string>
#include "Net/Network.h"
#include "Net/SocketAddress.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct addrinfo;

namespace traktor
{
	namespace net
	{

/*! \brief IPv6 socket address.
 * \ingroup Net
 */
class T_DLLCLASS SocketAddressIPv6 : public SocketAddress
{
	T_RTTI_CLASS;

public:
	SocketAddressIPv6();

	SocketAddressIPv6(uint16_t port);

	SocketAddressIPv6(const std::wstring& host, uint16_t port);

	virtual ~SocketAddressIPv6();
	
	virtual bool valid() const T_OVERRIDE T_FINAL;

	virtual std::wstring getHostName() const T_OVERRIDE T_FINAL;

	const addrinfo* getAddrInfo(int socktype) const;
	
private:
#if !defined(__PS3__) && !defined(__PS4__) && !defined(__EMSCRIPTEN__) && !defined(__PNACL__) && !defined(_XBOX)
	addrinfo* m_info;
#endif
};
	
	}
}

#endif	// traktor_net_SocketAddressIPv6_H
