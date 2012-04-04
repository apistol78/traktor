#ifndef traktor_parade_MessageRecorder_H
#define traktor_parade_MessageRecorder_H

#include "Core/Object.h"
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
#include "Parade/Network/Message.h"

namespace traktor
{

class IStream;
class Thread;

	namespace parade
	{

class MessageRecorder : public Object
{
	T_RTTI_CLASS;

public:
	MessageRecorder();

	bool create();

	void destroy();

	void addMessage(float time, uint32_t peerId, const Message& message);

private:
	struct Entry 
	{
		float time;
		uint32_t peerId;
		Message message;
	};

	Thread* m_threadWriter;
	Ref< IStream > m_stream;
	ThreadsafeFifo< Entry > m_writeQueue;
	Event m_writeQueueEvent;

	void threadWriter();
};

	}
}

#endif	// traktor_parade_MessageRecorder_H
