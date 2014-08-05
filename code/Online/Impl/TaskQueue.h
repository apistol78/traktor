#ifndef traktor_online_TaskQueue_H
#define traktor_online_TaskQueue_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"

namespace traktor
{

class Thread;

	namespace online
	{

class ISaveGame;
class ITask;

class TaskQueue : public Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	bool add(ITask* task);

private:
	Thread* m_thread;
	Semaphore m_queueLock;
	Signal m_queuedSignal;
	RefArray< ITask > m_queue;

	void flush();

	void threadQueue();
};

	}
}

#endif	// traktor_online_TaskQueue_H
