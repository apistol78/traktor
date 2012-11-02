#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/Replication/MessageRecorder.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.MessageRecorder", MessageRecorder, Object)

MessageRecorder::MessageRecorder()
:	m_threadWriter(0)
{
}

bool MessageRecorder::create()
{
	m_stream = FileSystem::getInstance().open(L"Messages.dump", File::FmWrite);
	if (!m_stream)
		return false;

	m_threadWriter = ThreadManager::getInstance().create(makeFunctor(this, &MessageRecorder::threadWriter), L"Write message thread");
	if (!m_threadWriter)
		return false;

	m_threadWriter->start();
	return true;
}

void MessageRecorder::destroy()
{
	if (m_threadWriter)
	{
		m_threadWriter->stop();
		m_threadWriter = 0;
	}
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}
}

void MessageRecorder::addMessage(float time, uint32_t peerId, const Message& message)
{
	Entry e = { time, peerId, message };
	m_writeQueue.put(e);
	m_writeQueueEvent.broadcast();
}

void MessageRecorder::threadWriter()
{
	Entry e;
	while (!m_threadWriter->stopped())
	{
		if (!m_writeQueueEvent.wait(100))
			continue;

		if (!m_writeQueue.get(e))
			continue;

		m_stream->write(&e, sizeof(e));
	}
}

	}
}
