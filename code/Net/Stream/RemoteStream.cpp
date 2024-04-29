/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <map>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Net/Batch.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/RemoteStream.h"

namespace traktor::net
{
	namespace
	{

class ConnectionPool : public ISingleton
{
public:
	static ConnectionPool& getInstance()
	{
		if (!ms_instance)
		{
			ms_instance = new ConnectionPool();
			SingletonManager::getInstance().add(ms_instance);
		}
		return *ms_instance;
	}

	TcpSocket* connect(const SocketAddressIPv4& addr)
	{
		// Try to get connection from pool.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			for (auto& entry : m_entries)
			{
				if (!entry.inuse && entry.addr.getAddr() == addr.getAddr() && entry.addr.getPort() == addr.getPort())
				{
					entry.inuse = true;
					return entry.socket;
				}
			}
		}

		// Create new connection.
		Ref< TcpSocket > socket = new TcpSocket();
		if (!socket->connect(addr))
			return nullptr;
		socket->setNoDelay(true);

		SocketEntry e;
		e.addr = addr;
		e.socket = socket;
		e.inuse = true;

		// Add to pool so we can reuse later.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_entries.push_back(e);
		}
		return socket;
	}

	void disconnect(TcpSocket* socket, bool failure)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			if (it->socket == socket)
			{
				T_ASSERT(it->inuse);
				if (!failure)
				{
					it->inuse = false;
				}
				else
				{
					it->socket->close();
					m_entries.erase(it);
				}
				return;
			}
		}
		T_FATAL_ERROR;
	}

protected:
	virtual void destroy()
	{
		T_ASSERT(ms_instance == this);
		delete ms_instance, ms_instance = nullptr;
	}

private:
	static ConnectionPool* ms_instance;

	struct SocketEntry
	{
		SocketAddressIPv4 addr;
		Ref< TcpSocket > socket;
		bool inuse = false;
	};

	Semaphore m_lock;
	std::list< SocketEntry > m_entries;
};

ConnectionPool* ConnectionPool::ms_instance = nullptr;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RemoteStream", RemoteStream, IStream)

Ref< IStream > RemoteStream::connect(const SocketAddressIPv4& addr, uint32_t id)
{
	Ref< TcpSocket > socket = ConnectionPool::getInstance().connect(addr);
	if (!socket)
	{
		log::error << L"RemoteStream; unable to connect to stream server." << Endl;
		return nullptr;
	}

	net::sendBatch< uint8_t, uint32_t >(socket, 0x01, id);

	uint8_t status = 0;
	int64_t avail = 0;

	if (net::recvBatch< uint8_t, int64_t >(socket, status, avail) <= 0)
	{
		log::error << L"RemoteStream; no status from server." << Endl;
		ConnectionPool::getInstance().disconnect(socket, true);
		return nullptr;
	}

	if (!status)
	{
		log::error << L"RemoteStream; invalid status from server." << Endl;
		ConnectionPool::getInstance().disconnect(socket, true);
		return nullptr;
	}

	if (avail > 0)
	{
		T_ASSERT((status & (0x01 | 0x02)) == 0x01);
		Ref< DynamicMemoryStream > dm = new DynamicMemoryStream(true, false);

		AlignedVector< uint8_t >& buffer = dm->getBuffer();
		buffer.resize(avail);

		uint8_t* ptr = &buffer[0];
		for (int64_t nread = 0; nread < avail; )
		{
			const int32_t read = int32_t(std::min< int64_t >(avail - nread, 1024 * 1024));
			const int32_t result = socket->recv(ptr, read);
			if (result <= 0)
			{
				log::error << L"RemoteStream; unexpected disconnect from server." << Endl;
				ConnectionPool::getInstance().disconnect(socket, true);
				return nullptr;
			}
			nread += result;
			ptr += result;
		}

		net::sendBatch< uint8_t >(socket, 0x02);
		ConnectionPool::getInstance().disconnect(socket, false);
		socket = nullptr;

		return dm;
	}

	Ref< RemoteStream > rs = new RemoteStream();
	rs->m_addr = addr;
	rs->m_socket = socket;
	rs->m_status = uint8_t(status);
	return rs;
}

RemoteStream::~RemoteStream()
{
	if (m_socket)
	{
		net::sendBatch< uint8_t >(m_socket, 0x02);
		ConnectionPool::getInstance().disconnect(m_socket, false);
		m_socket = nullptr;
	}
}

void RemoteStream::close()
{
	uint8_t result = 0;
	net::sendBatch< uint8_t >(m_socket, 0x03);
	net::recvBatch< uint8_t >(m_socket, result);
}

bool RemoteStream::canRead() const
{
	return (m_status & 0x01) == 0x01;
}

bool RemoteStream::canWrite() const
{
	return (m_status & 0x02) == 0x02;
}

bool RemoteStream::canSeek() const
{
	return (m_status & 0x04) == 0x04;
}

int64_t RemoteStream::tell() const
{
	int64_t result = 0;
	net::sendBatch< uint8_t >(m_socket, 0x04);
	net::recvBatch< int64_t >(m_socket, result);
	return result;
}

int64_t RemoteStream::available() const
{
	int64_t result = 0;
	net::sendBatch< uint8_t >(m_socket, 0x05);
	net::recvBatch< int64_t >(m_socket, result);
	return result;
}

int64_t RemoteStream::seek(SeekOriginType origin, int64_t offset)
{
	int64_t result = 0;
	net::sendBatch< uint8_t, int64_t, int64_t >(m_socket, 0x06, origin, offset);
	net::recvBatch< int64_t >(m_socket, result);
	return result;
}

int64_t RemoteStream::read(void* block, int64_t nbytes)
{
	int32_t ntotal = 0;

	if (net::sendBatch< uint8_t, int64_t >(m_socket, 0x07, nbytes) < 0)
		return -1;

	uint8_t* rp = (uint8_t*)block;
	while (nbytes > 0)
	{
		int64_t navail = 0;
		if (net::recvBatch< int64_t >(m_socket, navail) < 0)
			break;

		if (navail == 0 || navail > nbytes)
			break;
		if (navail < 0)
			return navail;

		int64_t nread = 0;
		while (nread < navail)
		{
			const int32_t result = m_socket->recv(rp, navail - nread);
			if (result > 0)
			{
				nread += result;
				rp += result;
			}
			else
				break;
		}
		if (nread != navail)
		{
			log::warning << L"Remote stream; didn't receive expected number of bytes (expected " << navail << L", got " << nread << L" byte(s)) from stream server." << Endl;
			break;
		}

		ntotal += navail;
		nbytes -= navail;
	}

	return ntotal;
}

int64_t RemoteStream::write(const void* block, int64_t nbytes)
{
	if (net::sendBatch< uint8_t, int64_t >(m_socket, 0x08, nbytes) < 0)
		return -1;

	const uint8_t* ptr = static_cast< const uint8_t* >(block);
	int64_t nwritten = 0;

	while (nwritten < nbytes)
	{
		const int32_t write = (int32_t)std::min< int64_t >(nbytes - nwritten, 65536);
		const int32_t result = m_socket->send(&ptr[nwritten], write);
		if (result != write)
			break;
		nwritten += write;
	}

	return nwritten;
}

void RemoteStream::flush()
{
	uint8_t result;
	net::sendBatch< uint8_t >(m_socket, 0x09);
	net::recvBatch< uint8_t >(m_socket, result);
}

RemoteStream::RemoteStream()
:	m_status(0)
{
}

}
