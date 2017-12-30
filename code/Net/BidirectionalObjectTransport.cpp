/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
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

BidirectionalObjectTransport::Result BidirectionalObjectTransport::recv(const TypeInfoSet& objectTypes, int32_t timeout, Ref< ISerializable >& outObject)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Check queue if any object of given type has already been received.
	for (TypeInfoSet::const_iterator i = objectTypes.begin(); i != objectTypes.end(); ++i)
	{
		RefArray< ISerializable >& typeInQueue = m_inQueue[*i];
		if (!typeInQueue.empty())
		{
			outObject = typeInQueue.front();
			typeInQueue.pop_front();
			return RtSuccess;
		}
	}

	if (m_socket)
	{
		// Receive objects from connection; if not of desired type then queue object.
		for (;;)
		{
			{
				T_ANONYMOUS_VAR(Release< Semaphore >)(m_lock);
				if (m_socket->select(true, false, false, timeout) <= 0)
					break;
			}
			if (m_socket->select(true, false, false, 0) <= 0)
				continue;

			Ref< ISerializable > object;
			{
				T_ANONYMOUS_VAR(Release< Semaphore >)(m_lock);
				net::SocketStream ss(m_socket, true, false, 10000);
				BinarySerializer s(&ss);
				object = s.readObject();
			}

			if (!object)
			{
				m_socket = 0;
				break;
			}

			for (TypeInfoSet::const_iterator i = objectTypes.begin(); i != objectTypes.end(); ++i)
			{
				if (is_type_of(*(*i), type_of(object)))
				{
					outObject = object;
					return RtSuccess;
				}
			}

			RefArray< ISerializable >& typeInQueue = m_inQueue[&type_of(object)];
			typeInQueue.push_back(object);
		}
	}

	if (!m_socket)
	{
		// If no connection return either timeout or disconnected, if any pending
		// object is queued then we timeout.
		for (SmallMap< const TypeInfo*, RefArray< ISerializable > >::const_iterator i = m_inQueue.begin(); i != m_inQueue.end(); ++i)
		{
			if (!i->second.empty())
				return RtTimeout;
		}
		return RtDisconnected;
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
