#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"
#include "Core/Timer/Timer.h"
#include "Net/Batch.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/StreamServer.h"

#define T_MEASURE_THROUGHPUT 0

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.StreamServer", StreamServer, Object)

StreamServer::StreamServer()
:	m_listenPort(0)
,	m_serverThread(0)
,	m_nextId(1)
{
}

bool StreamServer::create(uint16_t listenPort)
{
	m_listenSocket = new TcpSocket();
	if (!m_listenSocket->bind(SocketAddressIPv4(listenPort)))
		return false;
	if (!m_listenSocket->listen())
		return false;

	m_listenPort = listenPort;

	m_serverThread = ThreadManager::getInstance().create(
		makeFunctor(this, &StreamServer::threadServer),
		L"Stream server"
	);
	if (!m_serverThread)
		return false;

	m_serverThread->start();

	log::info << L"Stream server @" << listenPort << L" created" << Endl;
	return true;
}

void StreamServer::destroy()
{
	if (m_serverThread)
	{
		m_serverThread->stop();
		ThreadManager::getInstance().destroy(m_serverThread);
		m_serverThread = 0;
	}

	for (std::list< Thread* >::iterator i = m_clientThreads.begin(); i != m_clientThreads.end(); ++i)
		ThreadPool::getInstance().stop(*i);

	m_clientThreads.clear();
	m_streams.clear();
}

uint32_t StreamServer::publish(IStream* stream)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
	uint32_t id = m_nextId;
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

void StreamServer::threadServer()
{
	while (!m_serverThread->stopped())
	{
		if (m_listenSocket->select(true, false, false, 100) > 0)
		{
			Ref< TcpSocket > clientSocket = m_listenSocket->accept();
			if (!clientSocket)
			{
				log::error << L"StreamServer; unable to accept client" << Endl;
				continue;
			}

			clientSocket->setNoDelay(true);

			Thread* clientThread = ThreadPool::getInstance().spawn(
				makeFunctor< StreamServer, Ref< TcpSocket > >(this, &StreamServer::threadClient, clientSocket)
			);
			if (!clientThread)
			{
				log::error << L"StreamServer; unable to allocate client thread" << Endl;
				continue;
			}

			m_clientThreads.push_back(clientThread);
		}
	}
}

void StreamServer::threadClient(Ref< TcpSocket > clientSocket)
{
#pragma pack(1)
	struct { int32_t size; uint8_t data[65536]; } buffer;
#pragma pack()
	Ref< IStream > stream;
	uint32_t streamId;
	Timer timer;

	timer.start();

	double start = 0.0;
	int32_t totalRx = 0;
	int32_t totalTx = 0;
	int32_t countRx = 0;
	int32_t countTx = 0;

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped())
	{
		int32_t result = clientSocket->select(true, false, false, 100);
		if (result == 0)
			continue;
		else if (result < 0)
			break;

		uint8_t command;
		if (net::recvBatch< uint8_t >(clientSocket, command) <= 0)
			break;

		switch (command)
		{
		case 0x01:	// Acquire stream.
			{
				net::recvBatch< uint32_t >(clientSocket, streamId);

				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
					std::map< uint32_t, Ref< IStream > >::const_iterator i = m_streams.find(streamId);
					if (i != m_streams.end())
						stream = i->second;
					else
						stream = 0;
				}

				if (stream != 0)
				{
					uint8_t status = 0x00;
					if (stream->canRead())
						status |= 0x01;
					if (stream->canWrite())
						status |= 0x02;
					if (stream->canSeek())
						status |= 0x04;

					int32_t avail = 0;
					if ((status & 0x03) == 0x01)
						avail = stream->available();

					net::sendBatch< uint8_t, int32_t >(clientSocket, status, avail);

					start = timer.getElapsedTime();
					totalRx = 0;
					totalTx = 0;
					countRx = 0;
					countTx = 0;

					if (avail > 0)
					{
						SocketStream ss(clientSocket, false, true);
						StreamCopy(&ss, stream).execute(avail);
						totalTx += avail;
						countTx++;
					}
				}
				else
					net::sendBatch< uint8_t >(clientSocket, 0);
			}
			break;

		case 0x02:	// Release stream.
			{
				if (stream)
				{
					{
						T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_streamsLock);
						std::map< uint32_t, Ref< IStream > >::iterator i = m_streams.find(streamId);
						if (i != m_streams.end())
							m_streams.erase(i);
					}

					double end = timer.getElapsedTime();

#if T_MEASURE_THROUGHPUT
					log::info << L"Stream " << streamId << L", duration " << int32_t((end - start) * 1000) << L" ms" << Endl;
					log::info << L"RX " << totalRx << L" -- " << int32_t(totalRx / (end - start)) << L" bytes/s (" << countRx << L")" << Endl;
					log::info << L"TX " << totalTx << L" -- " << int32_t(totalTx / (end - start)) << L" bytes/s (" << countTx << L")" << Endl;
#endif

					stream = 0;
					streamId = 0;
				}
			}
			break;

		case 0x03:	// Close
			{
				if (stream)
				{
					stream->close();
					net::sendBatch< uint8_t >(clientSocket, 1);
				}
				else
					net::sendBatch< uint8_t >(clientSocket, 0);
			}
			break;

		case 0x04:	// Tell
			{
				if (stream)
					net::sendBatch< int32_t >(clientSocket, stream->tell());
				else
					net::sendBatch< int32_t >(clientSocket, -1);
			}
			break;

		case 0x05:	// Available
			{
				if (stream)
					net::sendBatch< int32_t >(clientSocket, stream->available());
				else
					net::sendBatch< int32_t >(clientSocket, -1);
			}
			break;

		case 0x06:	// Seek
			{
				if (stream)
				{
					int32_t origin, offset;
					net::recvBatch< int32_t, int32_t >(clientSocket, origin, offset);
					int32_t result = stream->seek((IStream::SeekOriginType)origin, offset);
					net::sendBatch< int32_t >(clientSocket, result);
				}
			}
			break;

		case 0x07:	// Read
			{
				if (stream)
				{
					int32_t nrequest;
					net::recvBatch< int32_t >(clientSocket, nrequest);

					while (nrequest > 0)
					{
						int32_t navail = min< int32_t >(nrequest, sizeof(buffer.data));
						int32_t nread = stream->read(buffer.data, navail);

						if (nread > 0)
						{
							buffer.size = nread;
							clientSocket->send(&buffer, sizeof(int32_t) + nread);
						}
						else
						{
							clientSocket->send(&nread, sizeof(int32_t));
							break;
						}

						nrequest -= nread;
						totalRx += nread;
					}

					++countRx;
				}
			}
			break;

		case 0x08:	// Write
			{
				if (stream)
				{
					int32_t nbytes;
					net::recvBatch< int32_t >(clientSocket, nbytes);

					while (nbytes > 0)
					{
						int32_t nread = min< int32_t >(nbytes, sizeof(buffer.data));
						int32_t nrecv = clientSocket->recv(buffer.data, nread);
						if (nrecv <= 0)
							break;

						stream->write(buffer.data, nrecv);

						totalTx += nrecv;
						nbytes -= nrecv;
					}

					++countTx;
				}
			}
			break;

		case 0x09:	// Flush
			{
				if (stream)
				{
					stream->flush();
					net::sendBatch< uint8_t >(clientSocket, 1);
				}
				else
					net::sendBatch< uint8_t >(clientSocket, 0);
			}
			break;
		}
	}
}

	}
}
