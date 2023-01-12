/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Socket.h"
#include "Net/SocketStream.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BidirectionalObjectTransport", BidirectionalObjectTransport, Object)

BidirectionalObjectTransport::BidirectionalObjectTransport(Socket* socket)
:	m_socket(socket)
{
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

	// Serialize into a memory blob.
	ChunkMemory memory;
	ChunkMemoryStream ms(&memory, false, true);
	if (!BinarySerializer(&ms).writeObject(object))
		return false;

	// Send entire blob in one go.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		for (size_t offset = 0; offset < memory.size(); )
		{
			const auto chunk = memory.getChunk(offset);
			if (!chunk.ptr)
				break;

			const int32_t nsent = m_socket->send(chunk.ptr, (int)chunk.size);
			if (nsent < 0)
				return false;

			offset += nsent;
		}
	}

	return true;
}

BidirectionalObjectTransport::Result BidirectionalObjectTransport::recv(const TypeInfoSet& objectTypes, int32_t timeout, Ref< ISerializable >& outObject)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Check queue if any object of given type has already been received.
	for (const auto& objectType : objectTypes)
	{
		RefArray< ISerializable >& typeInQueue = m_inQueue[objectType];
		if (!typeInQueue.empty())
		{
			outObject = typeInQueue.front();
			typeInQueue.pop_front();
			return Result::Success;
		}
	}

	if (m_socket)
	{
		// Receive objects from connection; if not of desired type then queue object.
		do
		{
			{
				T_ANONYMOUS_VAR(Release< Semaphore >)(m_lock);
				if (m_socket->select(true, false, false, timeout) <= 0)
					break;
			}

			// Check queue again if any object of given type has already been received on another thread.
			for (const auto& objectType : objectTypes)
			{
				RefArray< ISerializable >& typeInQueue = m_inQueue[objectType];
				if (!typeInQueue.empty())
				{
					outObject = typeInQueue.front();
					typeInQueue.pop_front();
					return Result::Success;
				}
			}

			// Ensure still have pending data on socket.
			if (m_socket->select(true, false, false, 0) <= 0)
				continue;

			// Receive object from socket.
			net::SocketStream ss(m_socket, true, false);
			Ref< ISerializable > object = BinarySerializer(&ss).readObject();
			if (!object)
			{
				m_socket = nullptr;
				break;
			}

			// Check if received object is of desired type.
			for (const auto& objectType : objectTypes)
			{
				if (is_type_of(*objectType, type_of(object)))
				{
					outObject = object;
					return Result::Success;
				}
			}

			// Received an object by which type we don't wait for; enqueue for later.
			m_inQueue[&type_of(object)].push_back(object);
		}
		while (timeout > 0);
	}

	if (!m_socket)
	{
		// If no connection return either timeout or disconnected, if any pending
		// object is queued then we timeout.
		for (const auto& it : m_inQueue)
		{
			if (!it.second.empty())
				return Result::Timeout;
		}
		return Result::Disconnected;
	}

	return Result::Timeout;
}

void BidirectionalObjectTransport::flush(const TypeInfo& objectType)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_inQueue[&objectType].resize(0);
}

}
