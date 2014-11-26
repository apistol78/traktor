#include "Core/Io/MemoryStream.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Thread/Acquire.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const uint32_t c_maxObjectSize = 10 * 1024 * 1024;

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

	uint8_t* buffer = static_cast< uint8_t* >(m_threadBuffer.get());
	if (!buffer)
	{
		buffer = new uint8_t [4 + c_maxObjectSize];
		m_buffers.push_back(buffer);
		m_threadBuffer.set(buffer);
	}

	MemoryStream dms(buffer + 4, c_maxObjectSize, false, true);
	if (!CompactSerializer(&dms, 0, 0).writeObject(object))
		return false;

	uint32_t objectSize = dms.tell();
	if (objectSize >= c_maxObjectSize)
		return false;

	*(uint32_t*)buffer = objectSize;
#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint32_t*)buffer);
#endif

	uint8_t* sendPtr = buffer;
	int32_t sendCount = 4 + objectSize;
	int32_t result = 0;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		while (sendCount > 0)
		{
			result = m_socket->send(sendPtr, sendCount);
			if (result <= 0 || result > sendCount)
				break;

			sendPtr += result;
			sendCount -= result;
		}
	}

	if (result <= 0)
	{
		m_socket = 0;
		return false;
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
		uint32_t objectSize = 0;
		int32_t result = m_socket->recv(&objectSize, 4);
		if (result <= 0)
		{
			m_socket = 0;
			m_inQueue.clear();
			return RtDisconnected;
		}

#if defined(T_BIG_ENDIAN)
		swap8in32(objectSize);
#endif
		if (result != 4 || objectSize == 0)
			continue;

		uint8_t* buffer = static_cast< uint8_t* >(m_threadBuffer.get());
		if (!buffer)
		{
			buffer = new uint8_t [4 + c_maxObjectSize];
			m_buffers.push_back(buffer);
			m_threadBuffer.set(buffer);
		}

		uint8_t* recvPtr = buffer;
		int32_t recvCount = objectSize;

		while (recvCount > 0)
		{
			result = m_socket->recv(recvPtr, recvCount);
			if (result <= 0 || result > recvCount)
				break;

			recvPtr += result;
			recvCount -= result;
		}

		if (result <= 0)
		{
			m_socket = 0;
			m_inQueue.clear();
			return RtDisconnected;
		}

		MemoryStream ms(buffer, objectSize, true, false);
		CompactSerializer s(&ms, 0, 0);

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
