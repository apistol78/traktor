#ifndef traktor_db_Connection_H
#define traktor_db_Connection_H

#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Remote/Messages/MsgStatus.h"

namespace traktor
{
	namespace net
	{

class TcpSocket;

	}

	namespace db
	{

class MessageTransport;

/*! \brief Database connection.
 * \ingroup Database
 */
class Connection : public Object
{
	T_RTTI_CLASS;

public:
	Connection(net::TcpSocket* socket);

	void destroy();

	template < typename ReplyMessageType >
	Ref< ReplyMessageType > sendMessage(const IMessage& message)
	{
		Ref< IMessage > reply = sendMessage(message);
		return dynamic_type_cast< ReplyMessageType* >(reply);
	}

private:
	Ref< net::TcpSocket > m_socket;
	Ref< MessageTransport > m_messageTransport;
	Semaphore m_messageLock;

	Ref< IMessage > sendMessage(const IMessage& message);
};

	}
}

#endif	// traktor_db_Connection_H
