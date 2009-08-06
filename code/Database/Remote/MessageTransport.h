#ifndef traktor_db_MessageTransport_H
#define traktor_db_MessageTransport_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace net
	{

class TcpSocket;
class SocketStream;

	}

	namespace db
	{

class IMessage;

/*! \brief
 */
class MessageTransport : public Object
{
	T_RTTI_CLASS(MessageTransport)

public:
	MessageTransport(net::TcpSocket* socket);

	bool send(const IMessage* message);

	IMessage* receive(int32_t timeout);

private:
	Ref< net::TcpSocket > m_socket;
	Ref< net::SocketStream > m_socketStream;
};

	}
}

#endif	// traktor_db_MessageTransport_H
