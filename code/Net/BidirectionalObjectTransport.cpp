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
}

bool BidirectionalObjectTransport::send(const ISerializable* object)
{
	if (m_socket)
	{
		SocketStream ss(m_socket, false, true);
		BinarySerializer s(&ss);
		if (s.writeObject(object))
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
		SocketStream ss(m_socket, true, false);
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
		else
		{
			m_socket = 0;
			m_inQueue.clear();
			return RtDisconnected;
		}
	}

	return RtTimeout;
}

	}
}
