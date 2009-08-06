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
	log::debug << L"Sent " << int32_t(buffer.size()) << L" byte(s)" << Endl;
#endif

	return true;
}

IMessage* MessageTransport::receive(int32_t timeout)
{
	int32_t result;

	result = m_socket->select(true, false, false, timeout);
	if (result <= 0)
		return 0;

	m_socketStream->setAccess(true, false);

#if defined(_DEBUG)
	log::debug << L"Receive pending " << m_socketStream->available() << Endl;
	int offset1 = m_socketStream->tell();
#endif

	Ref< IMessage > message = BinarySerializer(m_socketStream).readObject< IMessage >();

#if defined(_DEBUG)
	int offset2 = m_socketStream->tell();
	log::debug << L"Received " << (offset2 - offset1) << L" byte(s)" << Endl;
#endif

	return message;
}

	}
}
