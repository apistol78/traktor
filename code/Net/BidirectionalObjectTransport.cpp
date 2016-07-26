#include "Core/Io/BufferedStream.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BidirectionalObjectTransport", BidirectionalObjectTransport, Object)

BidirectionalObjectTransport::BidirectionalObjectTransport(TcpSocket* socket)
:	m_socket(socket)
{
}

BidirectionalObjectTransport::~BidirectionalObjectTransport()
{
	for (std::vector< uint8_t* >::iterator i = m_buffers.begin(); i != m_buffers.end(); ++i)
		delete[] *i;
}

void BidirectionalObjectTransport::close()
{
	safeClose(m_socket);
}

bool BidirectionalObjectTransport::send(const ISerializable* object)
{
	if (!m_socket)
		return false;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		net::SocketStream ss(m_socket, false, true, 10000);
		BufferedStream bs(&ss);
		if (!BinarySerializer(&bs).writeObject(object))
			return false;
		bs.flush();
	}

	return true;
}

bool BidirectionalObjectTransport::wait(int32_t timeout)
{
	if (m_socket)
		return m_socket->select(true, false, false, timeout) > 0;
	else
		return false;
}

BidirectionalObjectTransport::Result BidirectionalObjectTransport::recv(const TypeInfo& objectType, int32_t timeout, Ref< ISerializable >& outObject)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_socket)
		return RtDisconnected;

	// Check queue if any object of given type has already been received.
	RefArray< ISerializable >& typeInQueue = m_inQueue[&objectType];
	if (!typeInQueue.empty())
	{
		outObject = typeInQueue.front();
		typeInQueue.pop_front();
		return RtSuccess;
	}

	// Receive objects from connection; if not of desired type then queue object.
	while (m_socket->select(true, false, false, timeout) > 0)
	{
		net::SocketStream ss(m_socket, true, false, 10000);
		if (ss.available() <= 0)
		{
			m_socket = 0;
			m_inQueue.clear();
			return RtDisconnected;
		}

		BinarySerializer s(&ss);
		Ref< ISerializable > object = s.readObject();
		if (object)
		{
			if (is_type_of(objectType, type_of(object)))
			{
				outObject = object;
				return RtSuccess;
			}
			else
			{
				RefArray< ISerializable >& typeInQueue = m_inQueue[&type_of(object)];
				typeInQueue.push_back(object);
			}
		}
	}

	return RtTimeout;
}

void BidirectionalObjectTransport::flush(const TypeInfo& objectType)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	{
		RefArray< ISerializable >& typeInQueue = m_inQueue[&objectType];
		typeInQueue.clear();
	}
}

	}
}
