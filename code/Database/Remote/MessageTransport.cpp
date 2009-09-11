#include "Database/Remote/MessageTransport.h"
#include "Database/Remote/IMessage.h"
#include "Net/TcpSocket.h"
#include "Net/SocketStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const uint32_t c_averageMessageSize = 1024;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.MessageTransport", MessageTransport, Object)

MessageTransport::MessageTransport(net::TcpSocket* socket)
:	m_socket(socket)
,	m_buffer(c_averageMessageSize)
{
	m_socketStream = gc_new< net::SocketStream >(socket);
}

bool MessageTransport::send(const IMessage* message)
{
	m_buffer.resize(0);

	DynamicMemoryStream dms(m_buffer, false, true);
	if (!BinarySerializer(&dms).writeObject(message))
		return false;

	uint32_t bufferSize = uint32_t(m_buffer.size());
	if (!bufferSize)
		return false;

	m_socketStream->setAccess(false, true);
	if (m_socketStream->write(&m_buffer[0], bufferSize) != bufferSize)
		return false;

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
