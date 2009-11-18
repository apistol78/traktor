#ifndef traktor_net_SocketSet_H
#define traktor_net_SocketSet_H

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"

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

class Socket;

class T_DLLCLASS SocketSet : public Object
{
	T_RTTI_CLASS;

public:
	void add(Socket* socket);

	void remove(Socket* socket);

	int count() const;

	Ref< Socket > get(int index) const;

	int select(bool read, bool write, bool except, int timeout, SocketSet& outResultSet);

private:
	RefArray< Socket > m_sockets;
};

	}
}

#endif	// traktor_net_SocketSet_H
