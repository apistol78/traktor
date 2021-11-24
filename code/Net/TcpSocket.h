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

/*! TCP stream socket.
 * \ingroup Net
 */
class T_DLLCLASS TcpSocket : public Socket
{
	T_RTTI_CLASS;

public:
	TcpSocket();

	explicit TcpSocket(handle_t socket_);

	/*! Bind to local address. */
	bool bind(const SocketAddressIPv4& socketAddress, bool reuseAddr = false);

	/*! Bind to local address. */
	bool bind(const SocketAddressIPv6& socketAddress, bool reuseAddr = false);

	/*! Connect to remote host. */
	bool connect(const SocketAddressIPv4& socketAddress);

	/*! Connect to remote host. */
	bool connect(const SocketAddressIPv6& socketAddress);

	/*! Begin listen for connection requests. */
	bool listen();

	/*! Accept connection. */
	Ref< TcpSocket > accept();

	/*! Get local socket address. */
	Ref< SocketAddress > getLocalAddress();

	/*! Get remote socket address. */
	Ref< SocketAddress > getRemoteAddress();

	/*! Set "no delay" option; ie disable Nagle algorithm. */
	void setNoDelay(bool noDelay);

	/*! Set "quick ack" option. */
	void setQuickAck(bool quickAck);
};

	}
}

