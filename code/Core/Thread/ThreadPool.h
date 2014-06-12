#ifndef traktor_ThreadPool_H
#define traktor_ThreadPool_H

#include "Core/Containers/StaticVector.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Functor;
class Thread;

/*! \brief Thread pool manager.
 * \ingroup Core
 */
class T_DLLCLASS ThreadPool : public ISingleton
{
public:
	static ThreadPool& getInstance();

	bool spawn(Functor* functor, Thread*& outThread);

	bool join(Thread* thread);

	bool stop(Thread* thread);

protected:
	virtual void destroy();

private:
	struct Worker
	{
		Thread* threadWorker;
		Event eventAttachWork;
		Event eventFinishedWork;
		Ref< Functor > functorWork;
		int32_t alive;
		int32_t busy;

		Worker()
		:	threadWorker(0)
		,	alive(1)
		,	busy(0)
		{
		}
	};

	friend class StaticVector< Worker, 32 >;

	Semaphore m_lock;
	StaticVector< Worker, 32 > m_workerThreads;
};

}

#endif	// traktor_ThreadPool_H
