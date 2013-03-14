#include <map>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Net/Batch.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/RemoteStream.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

class ConnectionPool
{
public:
	static ConnectionPool& getInstance()
	{
		static ConnectionPool s_instance;
		return s_instance;
	}

	bool acquire(const SocketAddressIPv4& addr, Ref< TcpSocket >& outSocket)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		uint64_t key = (uint64_t(addr.getAddr()) << 16) | addr.getPort();
		
		std::map< uint64_t, RefArray< TcpSocket > >::iterator i = m_connections.find(key);
		if (i != m_connections.end() && !i->second.empty())
		{
			outSocket = i->second.back(); i->second.pop_back();
			return true;
		}

		Ref< TcpSocket > socket = new TcpSocket();
		if (!socket->connect(addr))
			return false;

		socket->setNoDelay(true);

		outSocket = socket;
		return true;
	}

	void release(const SocketAddressIPv4& addr, TcpSocket* socket)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		uint64_t key = (uint64_t(addr.getAddr()) << 16) | addr.getPort();
		m_connections[key].push_back(socket);
	}

private:
	Semaphore m_lock;
	std::map< uint64_t, RefArray< TcpSocket > > m_connections;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RemoteStream", RemoteStream, IStream)

Ref< IStream > RemoteStream::connect(const SocketAddressIPv4& addr, uint32_t id)
{
	Ref< TcpSocket > socket;
	if (!ConnectionPool::getInstance().acquire(addr, socket))
	{
		log::error << L"RemoteStream; unable to connect to stream server" << Endl;
		return 0;
	}

	net::sendBatch< uint8_t, uint32_t >(socket, 0x01, id);

	if (socket->select(true, false, false, 30000) <= 0)
	{
		log::error << L"RemoteStream; no response from server" << Endl;
		return 0;
	}

	uint8_t status;
	if (net::recvBatch< uint8_t >(socket, status) <= 0)
	{
		log::error << L"RemoteStream; invalid status " << status << L" from server" << Endl;
		return 0;
	}

	Ref< RemoteStream > rs = new RemoteStream();
	rs->m_addr = addr;
	rs->m_socket = socket;
	rs->m_status = uint8_t(status);

	if ((status & (0x01 | 0x02)) == 0x01)
	{
		int32_t avail = rs->available();
		if (avail > 0)
		{
			Ref< DynamicMemoryStream > dm = new DynamicMemoryStream(true, false, T_FILE_LINE);
		
			std::vector< uint8_t >& buffer = dm->getBuffer();
			buffer.resize(avail);

			if (rs->read(&buffer[0], avail) == avail)
				return dm;
			else
			{
				log::error << L"RemoteStream; unable to download stream" << Endl;
				return 0;
			}
		}
	}

	return new BufferedStream(rs);
}

RemoteStream::~RemoteStream()
{
	if (m_socket)
	{
		net::sendBatch< uint8_t >(m_socket, 0x02);
		ConnectionPool::getInstance().release(m_addr, m_socket);
		m_socket = 0;
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

int RemoteStream::tell() const
{
	int32_t result = 0;
	net::sendBatch< uint8_t >(m_socket, 0x04);
	net::recvBatch< int32_t >(m_socket, result);
	return result;
}

int RemoteStream::available() const
{
	int32_t result = 0;
	net::sendBatch< uint8_t >(m_socket, 0x05);
	net::recvBatch< int32_t >(m_socket, result);
	return result;
}

int RemoteStream::seek(SeekOriginType origin, int offset)
{
	int32_t result = 0;
	net::sendBatch< uint8_t, int32_t, int32_t >(m_socket, 0x06, origin, offset);
	net::recvBatch< int32_t >(m_socket, result);
	return result;
}

int RemoteStream::read(void* block, int nbytes)
{
	int32_t ntotal = 0;

	net::sendBatch< uint8_t, int32_t >(m_socket, 0x07, nbytes);

	uint8_t* rp = (uint8_t*)block;
	while (nbytes > 0)
	{
		int32_t navail = 0;
		net::recvBatch< int32_t >(m_socket, navail);

		if (navail == 0 || navail > nbytes)
			break;
		if (navail < 0)
			return navail;

		int32_t nread = 0;
		while (nread < navail)
		{
			int32_t result = m_socket->recv(rp, navail - nread);
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
			log::warning << L"Remote stream; didn't receive expected number of bytes (expected " << navail << L", got " << nread << L" byte(s)) from stream server" << Endl;
			break;
		}

		ntotal += navail;
		nbytes -= navail;
	}

	return ntotal;
}

int RemoteStream::write(const void* block, int nbytes)
{
	net::sendBatch< uint8_t, int32_t >(m_socket, 0x08, nbytes);
	return m_socket->send(block, nbytes);
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
}