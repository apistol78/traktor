/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"

namespace traktor
{

class Semaphore;

	namespace net
	{

class BidirectionalObjectTransport;
class StreamServer;
class TcpSocket;

	}

	namespace db
	{

class IMessage;
class IMessageListener;
class IProviderDatabase;

/*! Client connection.
 * \ingroup Database
 */
class Connection : public Object
{
	T_RTTI_CLASS;

public:
	Connection(
		Semaphore& connectionStringsLock,
		const SmallMap< std::wstring, std::wstring >& connectionStrings,
		net::StreamServer* streamServer,
		net::TcpSocket* clientSocket
	);

	void destroy();

	bool process();

	void sendReply(const IMessage& message);

	uint32_t putObject(Object* object);

	Object* getObject(uint32_t handle);

	void releaseObject(uint32_t handle);

	void setDatabase(IProviderDatabase* database);

	IProviderDatabase* getDatabase() const;

	net::StreamServer* getStreamServer() const;

	net::TcpSocket* getSocket() const;

	template < typename ObjectType >
	Ref< ObjectType > getObject(uint32_t handle)
	{
		return dynamic_type_cast< ObjectType* >(getObject(handle));
	}

private:
	Ref< net::StreamServer > m_streamServer;
	Ref< net::TcpSocket > m_clientSocket;
	Ref< net::BidirectionalObjectTransport > m_transport;
	RefArray< IMessageListener > m_messageListeners;
	SmallMap< uint32_t, Ref< Object > > m_objectStore;
	uint32_t m_nextHandle;
	Ref< IProviderDatabase > m_database;

	void messageThread();
};

	}
}
