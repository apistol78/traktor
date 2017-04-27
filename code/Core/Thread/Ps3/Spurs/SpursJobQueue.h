/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SpursJobQueue_H
#define traktor_SpursJobQueue_H

#include <cell/spurs.h>
#include "Core/Object.h"
#include "Core/Thread/IWaitable.h"
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

/*! \brief Spurs job manager.
 * \ingroup Core
 */
class T_DLLCLASS SpursJobQueue
:	public Object
,	public IWaitable
{
	T_RTTI_CLASS;

public:
	void destroy();

	bool push(CellSpursJobHeader* job);

	virtual bool wait(int32_t timeout = -1);

	template < typename JobHeaderType >
	bool push(JobHeaderType* job)
	{
		T_ASSERT (sizeof(JobHeaderType) == m_descriptorSize);
		return push(reinterpret_cast< CellSpursJobHeader* >(job));
	}

private:
	friend class SpursManager;

	CellSpurs* m_spurs;
	uint64_t* m_commandQueue;
	CellSpursJobHeader* m_descriptorBuffer;
	uint32_t m_descriptorSize;
	CellSpursJobQueue* m_jobQueue;
	CellSpursJobQueuePort* m_jobQueuePort;
	Semaphore m_lock;

	SpursJobQueue(CellSpurs* spurs);

	virtual ~SpursJobQueue();

	bool create(uint32_t descriptorSize, uint32_t submitCount, int priority);
};

}

#endif	// traktor_SpursJobQueue_H
