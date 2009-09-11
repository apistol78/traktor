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

/*! \brief Network message transportation.
 * \ingroup Database
 */
class MessageTransport : public Object
{
	T_RTTI_CLASS(MessageTransport)

public:
	MessageTransport(net::TcpSocket* socket);

	bool send(const IMessage* message);

	bool receive(int32_t timeout, Ref< IMessage >& outMessage);

private:
	Ref< net::TcpSocket > m_socket;
	Ref< net::SocketStream > m_socketStream;
	std::vector< uint8_t > m_buffer;
};

	}
}

#endif	// traktor_db_MessageTransport_H
