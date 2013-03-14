#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
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
	m_buffer.reset(new uint8_t [4 + 262144]);
}

void BidirectionalObjectTransport::close()
{
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}
}

bool BidirectionalObjectTransport::send(const ISerializable* object)
{
	if (m_socket)
	{
		MemoryStream dms(m_buffer.ptr() + 4, 262144, false, true);
		if (!BinarySerializer(&dms).writeObject(object))
			return false;

		uint32_t objectSize = dms.tell();
		if (objectSize >= 262144)
		{
			log::error << L"BidirectionalObjectTransport failed; object too big" << Endl;
			return false;
		}

		*(uint32_t*)m_buffer.ptr() = objectSize;

		if (m_socket->send(m_buffer.ptr(), 4 + objectSize) == 4 + objectSize)
			return true;
		else
		{
			m_socket = 0;
			return false;
		}
	}
	else
		return false;
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
		if (result != 4 || objectSize == 0)
			continue;

		if (net::SocketStream(m_socket, true, false).read(m_buffer.ptr(), objectSize) != objectSize)
		{
			m_socket = 0;
			m_inQueue.clear();
			return RtDisconnected;
		}

		MemoryStream ms(m_buffer.ptr(), objectSize, true, false);
		BinarySerializer s(&ms);

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
