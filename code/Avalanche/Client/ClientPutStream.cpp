#include "Avalanche/Protocol.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Client/ClientPutStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Thread/Acquire.h"
#include "Net/SocketStream.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.ClientPutStream", ClientPutStream, IStream)

ClientPutStream::ClientPutStream(Client* client, net::SocketStream* stream)
:	m_client(client)
,	m_stream(stream)
{
}

void ClientPutStream::close()
{
	if (!m_stream)
		return;

	const uint8_t cmd = c_subCommandPutCommit;
	if (m_stream->write(&cmd, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		m_stream = nullptr;
		return;
	}

	uint8_t reply = 0;
	if (m_stream->read(&reply, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		m_stream = nullptr;
		return;
	}

	if (reply == c_replyOk || reply == c_replyFailure)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_client->m_lock);
		m_client->m_streams.push_back(m_stream);
	}

	m_stream = nullptr;
}

bool ClientPutStream::canRead() const
{
	return false;
}

bool ClientPutStream::canWrite() const
{
	return true;
}

bool ClientPutStream::canSeek() const
{
	return false;
}

int64_t ClientPutStream::tell() const
{
	return 0;
}

int64_t ClientPutStream::available() const
{
	return 0;
}

int64_t ClientPutStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t ClientPutStream::read(void* block, int64_t nbytes)
{
	return -1;
}

int64_t ClientPutStream::write(const void* block, int64_t nbytes)
{
	if (!m_stream)
		return -1;

	if (nbytes <= 0)
		return 0;

	const uint8_t cmd = c_subCommandPutAppend;
	if (m_stream->write(&cmd, sizeof(uint8_t)) != sizeof(uint8_t))
	{
		m_stream = nullptr;
		return -1;
	}
	if (m_stream->write(&nbytes, sizeof(int64_t)) != sizeof(int64_t))
	{
		m_stream = nullptr;
		return -1;
	}
	if (m_stream->write(block, nbytes) != nbytes)
	{
		m_stream = nullptr;
		return -1;
	}

	return nbytes;
}

void ClientPutStream::flush()
{
}

	}
}
