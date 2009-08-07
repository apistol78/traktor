#include "Database/Remote/MessageTransport.h"
#include "Database/Remote/IMessage.h"
#include "Net/TcpSocket.h"
#include "Net/SocketStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.MessageTransport", MessageTransport, Object)

MessageTransport::MessageTransport(net::TcpSocket* socket)
:	m_socket(socket)
{
	m_socketStream = gc_new< net::SocketStream >(socket);
}

bool MessageTransport::send(const IMessage* message)
{
	std::vector< uint8_t > buffer;

	DynamicMemoryStream dms(buffer, false, true);
	if (!BinarySerializer(&dms).writeObject(message))
		return false;

	m_socketStream->setAccess(false, true);

	if (m_socketStream->write(&buffer[0], int(buffer.size())) != int(buffer.size()))
		return false;

#if defined(_DEBUG)
	log::debug << L"Sent message " << type_name(message) << L" in " << int32_t(buffer.size()) << L" byte(s)" << Endl;
#endif

	return true;
}

bool MessageTransport::receive(int32_t timeout, Ref< IMessage >& outMessage)
{
	int32_t result;

	result = m_socket->select(true, false, false, timeout);
	if (result <= 0)
		return result == 0;
	
	m_socketStream->setAccess(true, false);
	outMessage = BinarySerializer(m_socketStream).readObject< IMessage >();

	return outMessage != 0;
}

	}
}
