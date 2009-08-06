#include "Database/Remote/Client/RemoteStream.h"
#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/Messages/StmClose.h"
#include "Database/Remote/Messages/StmGetStat.h"
#include "Database/Remote/Messages/StmSeek.h"
#include "Database/Remote/Messages/StmRead.h"
#include "Database/Remote/Messages/StmWrite.h"
#include "Database/Remote/Messages/StmStatResult.h"
#include "Database/Remote/Messages/StmReadResult.h"
#include "Database/Remote/Messages/StmWriteResult.h"
#include "Database/Remote/Messages/MsgIntResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteStream", RemoteStream, Stream)

RemoteStream::RemoteStream(Connection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

void RemoteStream::close()
{
	m_connection->sendMessage< MsgStatus >(StmClose(m_handle));
}

bool RemoteStream::canRead() const
{
	Ref< StmStatResult > result = m_connection->sendMessage< StmStatResult >(StmGetStat(m_handle));
	return result ? result->canRead() : false;
}

bool RemoteStream::canWrite() const
{
	Ref< StmStatResult > result = m_connection->sendMessage< StmStatResult >(StmGetStat(m_handle));
	return result ? result->canWrite() : false;
}

bool RemoteStream::canSeek() const
{
	Ref< StmStatResult > result = m_connection->sendMessage< StmStatResult >(StmGetStat(m_handle));
	return result ? result->canSeek() : false;
}

int RemoteStream::tell() const
{
	Ref< StmStatResult > result = m_connection->sendMessage< StmStatResult >(StmGetStat(m_handle));
	return result ? result->tell() : false;
}

int RemoteStream::available() const
{
	Ref< StmStatResult > result = m_connection->sendMessage< StmStatResult >(StmGetStat(m_handle));
	return result ? result->available() : false;
}

int RemoteStream::seek(SeekOriginType origin, int offset)
{
	Ref< MsgIntResult > result = m_connection->sendMessage< MsgIntResult >(StmSeek(m_handle, origin, offset));
	return result ? result->get() : 0;
}

int RemoteStream::read(void* block, int nbytes)
{
	uint8_t* blockPtr = static_cast< uint8_t* >(block);

	while (nbytes > 0)
	{
		int32_t blockSize = std::min< int32_t >(nbytes, StmRead::MaxBlockSize);

		Ref< StmReadResult > result = m_connection->sendMessage< StmReadResult >(StmRead(m_handle, blockSize));
		if (!result)
			return -1;

		int32_t readBlockSize = result->getBlockSize();
		if (readBlockSize <= 0)
			break;

		std::memcpy(blockPtr, result->getBlock(), readBlockSize);

		blockPtr += readBlockSize;
		nbytes -= readBlockSize;

		if (readBlockSize < StmRead::MaxBlockSize)
			break;
	}

	return static_cast< int >(blockPtr - static_cast< uint8_t* >(block));
}

int RemoteStream::write(const void* block, int nbytes)
{
	const uint8_t* blockPtr = static_cast< const uint8_t* >(block);

	while (nbytes > 0)
	{
		int32_t blockSize = std::min< int32_t >(nbytes, StmWrite::MaxBlockSize);

		Ref< StmWriteResult > result = m_connection->sendMessage< StmWriteResult >(StmWrite(m_handle, blockSize, blockPtr));
		if (!result)
			return -1;

		int32_t writeBlockSize = result->get();
		if (writeBlockSize <= 0)
			break;

		blockPtr += writeBlockSize;
		nbytes -= writeBlockSize;
	}
	T_ASSERT (nbytes >= 0);

	return static_cast< int >(blockPtr - static_cast< const uint8_t* >(block));
}

void RemoteStream::flush()
{
}

	}
}
