#ifndef traktor_db_MessageTransport_H
#define traktor_db_MessageTransport_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS TcpSocket;
class T_DLLCLASS SocketStream;

	}

	namespace db
	{

class T_DLLCLASS IMessage;

/*! \brief Network message transportation.
 * \ingroup Database
 */
class T_DLLCLASS MessageTransport : public Object
{
	T_RTTI_CLASS;

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
