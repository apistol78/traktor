/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class Socket;

/*!
 * \ingroup Net
 */
class T_DLLCLASS BidirectionalObjectTransport : public Object
{
	T_RTTI_CLASS;

public:
	enum class Result
	{
		Timeout,
		Success,
		Disconnected
	};

	explicit BidirectionalObjectTransport(Socket* socket);

	void close();

	bool send(const ISerializable* object);

	Result recv(const TypeInfoSet& objectTypes, int32_t timeout, Ref< ISerializable >& outObject);

	void flush(const TypeInfo& objectType);

	bool connected() const { return m_socket != 0; }

	Socket* getSocket() const { return m_socket; }

	template < typename ObjectType >
	Result recv(int32_t timeout, Ref< ObjectType >& outObject)
	{
		Ref< ISerializable > obj;
		Result result;

		result = recv(makeTypeInfoSet< ObjectType >(), timeout, obj);
		if (result != Result::Success)
			return result;

		outObject = dynamic_type_cast< ObjectType* >(obj);
		return Result::Success;
	}

	template < typename ObjectType >
	void flush()
	{
		flush(type_of< ObjectType >());
	}

private:
	Ref< Socket > m_socket;
	SmallMap< const TypeInfo*, RefArray< ISerializable > > m_inQueue;
	Semaphore m_lock;
};

}
