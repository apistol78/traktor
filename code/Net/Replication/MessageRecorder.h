#ifndef traktor_net_MessageRecorder_H
#define traktor_net_MessageRecorder_H

#include "Core/Object.h"
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
#include "Net/Replication/Message.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class Thread;

	namespace net
	{

class T_DLLCLASS MessageRecorder : public Object
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

#endif	// traktor_net_MessageRecorder_H
