/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteConnection", RemoteConnection, Object)

RemoteConnection::RemoteConnection(net::TcpSocket* socket)
:	m_socket(socket)
{
	m_transport = new net::BidirectionalObjectTransport(m_socket);
}

void RemoteConnection::destroy()
{
	if (m_transport)
		m_transport = nullptr;

	safeClose(m_socket);
}

void RemoteConnection::setStreamServerAddr(const net::SocketAddressIPv4& streamServerAddr)
{
	m_streamServerAddr = streamServerAddr;
}

const net::SocketAddressIPv4& RemoteConnection::getStreamServerAddr() const
{
	return m_streamServerAddr;
}

Ref< IMessage > RemoteConnection::sendMessage(const IMessage& message)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_transportLock);
	Ref< IMessage > reply;

	if (!m_transport || !m_transport->connected())
		return nullptr;

	m_transport->flush< IMessage >();

	if (!m_transport->send(&message))
		return nullptr;

	if (m_transport->recv(60000, reply) != net::BidirectionalObjectTransport::Result::Success)
		return nullptr;

	return reply;
}

	}
}
