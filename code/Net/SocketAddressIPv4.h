#ifndef traktor_net_SocketAddressIPv4_H
#define traktor_net_SocketAddressIPv4_H

#include <string>
#include "Net/Network.h"
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
	T_RTTI_CLASS(SocketAddressIPv4)

public:
	SocketAddressIPv4();

	SocketAddressIPv4(const sockaddr_in& sockaddr);
	
	SocketAddressIPv4(uint16_t port);

	SocketAddressIPv4(const uint8_t ip[4], uint16_t port);
	
	SocketAddressIPv4(const std::wstring& host, uint16_t port);
	
	bool valid() const;
	
	std::wstring getHostName() const;

	uint16_t getPort() const;

	void setSockAddr(const sockaddr_in& sockaddr);

	const sockaddr_in& getSockAddr() const;
	
private:
	sockaddr_in m_sockaddr;
};
	
	}
}

#endif	// traktor_net_SocketAddressIPv4_H
