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
		namespace
		{

struct ObjectTypePred
{
	const TypeInfo& m_objectType;

	ObjectTypePred(const TypeInfo& objectType)
	:	m_objectType(objectType)
	{
	}

	bool operator () (const ISerializable* object) const
	{
		return is_type_of(m_objectType, type_of(object));
	}
};

		}

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
	RefArray< ISerializable >::iterator i = std::find_if(m_inQueue.begin(), m_inQueue.end(), ObjectTypePred(objectType));
	if (i != m_inQueue.end())
	{
		outObject = *i;
		m_inQueue.erase(i);
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
			if (ObjectTypePred(objectType)(object))
			{
				outObject = object;
				return RtSuccess;
			}
			else
				m_inQueue.push_back(object);
		}
	}

	return RtTimeout;
}

void BidirectionalObjectTransport::flush(const TypeInfo& objectType)
{
	for (;;)
	{
		RefArray< ISerializable >::iterator i = std::find_if(m_inQueue.begin(), m_inQueue.end(), ObjectTypePred(objectType));
		if (i == m_inQueue.end())
			break;
		m_inQueue.erase(i);
	}
}

	}
}
