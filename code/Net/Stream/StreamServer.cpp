/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Net/Batch.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/StreamServer.h"

#define T_MEASURE_THROUGHPUT 0

namespace traktor::net
{
	namespace
	{

const int32_t c_preloadSmallStreamSize = 8 * 1024 * 1024;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.StreamServer", StreamServer, Object)

bool StreamServer::create()
{
	m_listenSocket = new TcpSocket();
	if (!m_listenSocket->bind(SocketAddressIPv4()))
		return false;
	if (!m_listenSocket->listen())
		return false;

	m_listenPort = dynamic_type_cast< net::SocketAddressIPv4* >(m_listenSocket->getLocalAddress())->getPort();

	m_serverThread = ThreadManager::getInstance().create([=, this](){ threadServer(); }, L"Stream server");
	if (!m_serverThread)
		return false;

	m_serverThread->start();

	log::info << L"Stream server @" << m_listenPort << L" created." << Endl;
	return true;
}

void StreamServer::destroy()
{
	if (m_serverThread)
	{
		m_serverThread->stop();
		ThreadManager::getInstance().destroy(m_serverThread);
		m_serverThread = nullptr;
	}

	m_streams.clear();
}

uint32_t StreamServer::publish(IStream* stream)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
	const uint32_t id = m_nextId;
	m_streams[id] = stream;
	if (++m_nextId == 0)
		m_nextId = 1;
	return id;
}

uint16_t StreamServer::getListenPort() const
{
	return m_listenPort;
}

TcpSocket* StreamServer::getListenSocket() const
{
	return m_listenSocket;
}

uint32_t StreamServer::getStreamCount() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
	return (uint32_t)m_streams.size();
}

void StreamServer::threadServer()
{
#pragma pack(1)
	struct { int64_t size; uint8_t data[65536]; } buffer;
#pragma pack()

	while (!m_serverThread->stopped())
	{
		SocketSet ss;
		ss.add(m_listenSocket);
		for (const auto& client : m_clients)
			ss.add(client.socket);

		SocketSet result;
		if (ss.select(true, false, false, 100, result) <= 0)
			continue;

		// Accept new clients.
		if (result.contain(m_listenSocket))
		{
			Ref< TcpSocket > clientSocket = m_listenSocket->accept();
			if (!clientSocket)
			{
				log::error << L"StreamServer; unable to accept client." << Endl;
				continue;
			}

			clientSocket->setNoDelay(true);

			auto& client = m_clients.push_back();
			client.socket = clientSocket;
			client.stream = nullptr;
			client.streamId = 0;
		}

		// Serve clients.
		for (int32_t i = 0; i < result.count(); ++i)
		{
			Ref< Socket > socket = result.get(i);
			if (socket == m_listenSocket)
				continue;

			auto it = std::find_if(m_clients.begin(), m_clients.end(), [&](const Client& client) {
				return client.socket == socket;
			});
			if (it == m_clients.end())
				continue;

			auto& client = *it;

			uint8_t command = 0x00;
			if (net::recvBatch< uint8_t >(client.socket, command) <= 0)
			{
				m_clients.erase(it);
				continue;
			}

			switch (command)
			{
			case 0x01:	// Acquire stream.
			case 0x81:	// Acquire stream (no pre-load).
				{
					net::recvBatch< uint32_t >(client.socket, client.streamId);

					{
						T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
						auto it = m_streams.find(client.streamId);
						if (it != m_streams.end())
							client.stream = it->second;
						else
							client.stream = nullptr;
					}

					if (client.stream != nullptr)
					{
						uint8_t status = 0x00;
						if (client.stream->canRead())
							status |= 0x01;
						if (client.stream->canWrite())
							status |= 0x02;
						if (client.stream->canSeek())
							status |= 0x04;

						int64_t avail = 0;
						if (command == 0x01 && (status & 0x03) == 0x01)
						{
							int64_t streamAvail = client.stream->available();
							if (streamAvail <= c_preloadSmallStreamSize)
								avail = streamAvail;
						}

						net::sendBatch< uint8_t, int64_t >(client.socket, status, avail);

						if (avail > 0)
						{
							SocketStream ss(client.socket, false, true);
							StreamCopy(&ss, client.stream).execute(avail);
						}
					}
					else
					{
						log::warning << L"Unable to serve remote stream; no such stream " << client.streamId << L"." << Endl;
						net::sendBatch< uint8_t, int64_t >(client.socket, 0, 0);
					}
				}
				break;

			case 0x02:	// Release stream.
				{
					if (client.stream)
					{
						{
							T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
							auto it = m_streams.find(client.streamId);
							if (it != m_streams.end())
								m_streams.erase(it);
						}

						client.stream = nullptr;
						client.streamId = 0;
					}
				}
				break;

			case 0x03:	// Close
				{
					if (client.stream)
					{
						client.stream->close();
						net::sendBatch< uint8_t >(client.socket, 1);
					}
					else
						net::sendBatch< uint8_t >(client.socket, 0);
				}
				break;

			case 0x04:	// Tell
				{
					if (client.stream)
						net::sendBatch< int64_t >(client.socket, client.stream->tell());
					else
						net::sendBatch< int64_t >(client.socket, -1);
				}
				break;

			case 0x05:	// Available
				{
					if (client.stream)
						net::sendBatch< int64_t >(client.socket, client.stream->available());
					else
						net::sendBatch< int64_t >(client.socket, -1);
				}
				break;

			case 0x06:	// Seek
				{
					if (client.stream)
					{
						int64_t origin = 0, offset = 0;
						net::recvBatch< int64_t, int64_t >(client.socket, origin, offset);
						int64_t resultSeek = client.stream->seek((IStream::SeekOriginType)origin, offset);
						net::sendBatch< int64_t >(client.socket, resultSeek);
					}
				}
				break;

			case 0x07:	// Read
				{
					if (client.stream)
					{
						int64_t nrequest = 0;
						net::recvBatch< int64_t >(client.socket, nrequest);

						while (nrequest > 0)
						{
							const int64_t navail = min< int64_t >(nrequest, sizeof(buffer.data));
							const int64_t nread = client.stream->read(buffer.data, navail);

							if (nread > 0)
							{
								buffer.size = nread;
								client.socket->send(&buffer, (int32_t)(sizeof(int64_t) + nread));
							}
							else
							{
								client.socket->send(&nread, sizeof(int64_t));
								break;
							}

							nrequest -= nread;
						}
					}
				}
				break;

			case 0x08:	// Write
				{
					if (client.stream)
					{
						int64_t nbytes = 0;
						net::recvBatch< int64_t >(client.socket, nbytes);

						while (nbytes > 0)
						{
							const int64_t nread = min< int64_t >(nbytes, sizeof(buffer.data));
							const int32_t nrecv = client.socket->recv(buffer.data, (int32_t)nread);
							if (nrecv <= 0)
								break;

							client.stream->write(buffer.data, nrecv);

							nbytes -= nrecv;
						}
					}
				}
				break;

			case 0x09:	// Flush
				{
					if (client.stream)
					{
						client.stream->flush();
						net::sendBatch< uint8_t >(client.socket, 1);
					}
					else
						net::sendBatch< uint8_t >(client.socket, 0);
				}
				break;

			default:
				log::error << L"Unknown stream command " << str(L"0x%02x", command) << L" from client." << Endl;
				break;
			}
		}
	}
}

}
