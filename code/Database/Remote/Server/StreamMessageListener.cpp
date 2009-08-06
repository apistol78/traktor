#include "Database/Remote/Server/StreamMessageListener.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Messages/StmClose.h"
#include "Database/Remote/Messages/StmGetStat.h"
#include "Database/Remote/Messages/StmSeek.h"
#include "Database/Remote/Messages/StmRead.h"
#include "Database/Remote/Messages/StmWrite.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/MsgIntResult.h"
#include "Database/Remote/Messages/StmStatResult.h"
#include "Database/Remote/Messages/StmReadResult.h"
#include "Database/Remote/Messages/StmWriteResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.StreamMessageListener", StreamMessageListener, IMessageListener)

StreamMessageListener::StreamMessageListener(Connection* connection)
:	m_connection(connection)
{
	registerMessage< StmClose >(&StreamMessageListener::messageClose);
	registerMessage< StmGetStat >(&StreamMessageListener::messageGetStat);
	registerMessage< StmSeek >(&StreamMessageListener::messageSeek);
	registerMessage< StmRead >(&StreamMessageListener::messageRead);
	registerMessage< StmWrite >(&StreamMessageListener::messageWrite);
}

bool StreamMessageListener::messageClose(const StmClose* message)
{
	uint32_t streamHandle = message->getHandle();
	Ref< Stream > stream = m_connection->getObject< Stream >(streamHandle);
	if (!stream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	stream->close();

	m_connection->releaseObject(streamHandle);
	m_connection->sendReply(MsgStatus(StSuccess));
	return true;
}

bool StreamMessageListener::messageGetStat(const StmGetStat* message)
{
	uint32_t streamHandle = message->getHandle();
	Ref< Stream > stream = m_connection->getObject< Stream >(streamHandle);
	if (!stream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->sendReply(StmStatResult(
		stream->canRead(),
		stream->canWrite(),
		stream->canSeek(),
		stream->tell(),
		stream->available()
	));
	return true;
}

bool StreamMessageListener::messageSeek(const StmSeek* message)
{
	uint32_t streamHandle = message->getHandle();
	Ref< Stream > stream = m_connection->getObject< Stream >(streamHandle);
	if (!stream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	int result = stream->seek(message->getOrigin(), message->getOffset());
	m_connection->sendReply(MsgIntResult(result));
	return true;
}

bool StreamMessageListener::messageRead(const StmRead* message)
{
	uint32_t streamHandle = message->getHandle();
	Ref< Stream > stream = m_connection->getObject< Stream >(streamHandle);
	if (!stream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint8_t block[StmRead::MaxBlockSize];
	int32_t result = stream->read(block, message->getBlockSize());

	m_connection->sendReply(StmReadResult(block, result));
	return true;
}

bool StreamMessageListener::messageWrite(const StmWrite* message)
{
	uint32_t streamHandle = message->getHandle();
	Ref< Stream > stream = m_connection->getObject< Stream >(streamHandle);
	if (!stream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	int32_t result = stream->write(message->getBlock(), message->getBlockSize());
	m_connection->sendReply(StmWriteResult(result));
	return true;
}

	}
}
