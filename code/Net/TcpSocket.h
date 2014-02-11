#ifndef traktor_net_TcpSocket_H
#define traktor_net_TcpSocket_H

#include "Net/Socket.h"

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

class SocketAddress;
class SocketAddressIPv4;
class SocketAddressIPv6;

/*! \brief TCP stream socket.
 * \ingroup Net
 */
class T_DLLCLASS TcpSocket : public Socket
{
	T_RTTI_CLASS;

public:
	TcpSocket();

	TcpSocket(SOCKET socket_);

	/*! \brief Bind to local address. */
	bool bind(const SocketAddressIPv4& socketAddress, bool reuseAddr = false);

	/*! \brief Bind to local address. */
	bool bind(const SocketAddressIPv6& socketAddress, bool reuseAddr = false);

	/*! \brief Connect to remote host. */
	bool connect(const SocketAddressIPv4& socketAddress);

	/*! \brief Connect to remote host. */
	bool connect(const SocketAddressIPv6& socketAddress);

	/*! \brief Begin listen for connection requests. */
	bool listen();

	/*! \brief Accept connection. */
	Ref< TcpSocket > accept();

	/*! \brief Get local socket address. */
	Ref< SocketAddress > getLocalAddress();

	/*! \brief Get remote socket address. */
	Ref< SocketAddress > getRemoteAddress();

	/*! \brief Set "no delay" option; ie disable Nagle algorithm. */
	void setNoDelay(bool noDelay);
};

	}
}

#endif	// traktor_net_TcpSocket_H
