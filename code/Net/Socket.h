#ifndef traktor_net_Socket_H
#define traktor_net_Socket_H

#include "Net/Network.h"
#include "Core/Object.h"

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

enum IoctlCommand
{
	IccNonBlockingIo,	// FIONBIO
	IccReadPending		// FIONREAD
};

class T_DLLCLASS Socket : public Object
{
	T_RTTI_CLASS(Socket)

public:
	Socket();

	Socket(SOCKET socket_);

	virtual ~Socket();

	void close();
	
	int select(bool read, bool write, bool except, int timeout);

	int send(const void* data, int length);
	
	int recv(void* data, int length);

	int send(int byte);

	int recv();
	
	bool ioctl(IoctlCommand cmd, unsigned long* argp);

	SOCKET handle() const;

protected:
	SOCKET m_socket;
};

	}
}

#endif	// traktor_net_Socket_H
