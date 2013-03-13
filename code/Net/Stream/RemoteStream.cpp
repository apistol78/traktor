#include <map>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
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

	Ref< TcpSocket > acquire(const SocketAddressIPv4& addr)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		uint64_t key = (uint64_t(addr.getAddr()) << 16) | addr.getPort();
		std::map< uint64_t, Ref< TcpSocket > >::iterator i = m_connections.find(key);
		if (i != m_connections.end())
		{
			Ref< TcpSocket > socket = i->second;
			m_connections.erase(i);
			return socket;
		}

		Ref< TcpSocket > socket = new TcpSocket();
		if (!socket->connect(addr))
			return 0;

		return socket;
	}

	void release(TcpSocket* socket)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		const SocketAddressIPv4& addr = *checked_type_cast< const SocketAddressIPv4*, false >(socket->getRemoteAddress());
		uint64_t key = (uint64_t(addr.getAddr()) << 16) | addr.getPort();
		m_connections[key] = socket;
	}

private:
	Semaphore m_lock;
	std::map< uint64_t, Ref< TcpSocket > > m_connections;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RemoteStream", RemoteStream, IStream)

Ref< IStream > RemoteStream::connect(const SocketAddressIPv4& addr, uint32_t id)
{
	Ref< TcpSocket > socket = ConnectionPool::getInstance().acquire(addr);
	if (!socket)
		return 0;

	Ref< SocketStream > socketStream = new SocketStream(socket);
	Ref< Reader > reader = new Reader(socketStream);
	Ref< Writer > writer = new Writer(socketStream);

	*writer << uint8_t(0x01);
	*writer << id;

	if (socket->select(true, false, false, 30000) <= 0)
		return 0;

	int32_t status = socket->recv();
	if (status < 0)
		return 0;

	if (status != 0)
	{
		Ref< RemoteStream > rs = new RemoteStream();
		rs->m_socket = socket;
		rs->m_reader = reader;
		rs->m_writer = writer;
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
					return 0;
			}
		}

		return new BufferedStream(rs);
	}
	else
		return 0;
}

RemoteStream::~RemoteStream()
{
	if (m_socket)
	{
		*m_writer << uint8_t(0x02);

		m_writer = 0;
		m_reader = 0;

		ConnectionPool::getInstance().release(m_socket);
		m_socket = 0;
	}
}

void RemoteStream::close()
{
	if (m_socket)
	{
		bool result;
		*m_writer << uint8_t(0x03);
		*m_reader >> result;
	}
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
	int32_t result;
	*m_writer << uint8_t(0x04);
	*m_reader >> result;
	return result;
}

int RemoteStream::available() const
{
	int32_t result;
	*m_writer << uint8_t(0x05);
	*m_reader >> result;
	return result;
}

int RemoteStream::seek(SeekOriginType origin, int offset)
{
	int32_t result;
	*m_writer << uint8_t(0x06);
	*m_writer << int32_t(origin);
	*m_writer << int32_t(offset);
	*m_reader >> result;
	return result;
}

int RemoteStream::read(void* block, int nbytes)
{
	int32_t ntotal = 0;

	*m_writer << uint8_t(0x07);
	*m_writer << int32_t(nbytes);

	uint8_t* rp = (uint8_t*)block;
	while (nbytes > 0)
	{
		int32_t navail = -1;
		*m_reader >> navail;

		if (navail == 0 || navail > nbytes)
			break;
		if (navail < 0)
			return navail;

		int32_t result = m_reader->read(rp, navail);
		if (result != navail)
		{
			log::warning << L"Remote stream; didn't receive expected number of bytes (" << navail << L") from stream server" << Endl;
			break;
		}

		ntotal += navail;
		nbytes -= navail;
		rp += navail;
	}

	return ntotal;
}

int RemoteStream::write(const void* block, int nbytes)
{
	*m_writer << uint8_t(0x08);
	*m_writer << int32_t(nbytes);
	return m_writer->write(block, nbytes);
}

void RemoteStream::flush()
{
	if (m_socket)
	{
		bool result;
		*m_writer << uint8_t(0x09);
		*m_reader >> result;
	}
}

RemoteStream::RemoteStream()
:	m_status(0)
{
}

	}
}