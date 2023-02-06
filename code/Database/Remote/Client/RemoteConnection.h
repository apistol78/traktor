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
#include "Core/Thread/Semaphore.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor::net
{

class BidirectionalObjectTransport;
class Socket;

}

namespace traktor::db
{

/*! Database connection.
 * \ingroup Database
 */
class RemoteConnection : public Object
{
	T_RTTI_CLASS;

public:
	explicit RemoteConnection(net::Socket* socket);

	void destroy();

	void setStreamServerAddr(const net::SocketAddressIPv4& streamServerAddr);

	const net::SocketAddressIPv4& getStreamServerAddr() const;

	template < typename ReplyMessageType >
	Ref< ReplyMessageType > sendMessage(const IMessage& message)
	{
		Ref< IMessage > reply = sendMessage(message);
		return dynamic_type_cast< ReplyMessageType* >(reply);
	}

private:
	Ref< net::Socket > m_socket;
	net::SocketAddressIPv4 m_streamServerAddr;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Semaphore m_transportLock;

	Ref< IMessage > sendMessage(const IMessage& message);
};

}
