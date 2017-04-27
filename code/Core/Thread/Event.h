/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Event_H
#define traktor_Event_H

#if defined(__PS3__)
#	include <sys/synchronization.h>
#elif defined(__PS4__)
#	include <kernel.h>
#endif
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Thread signal event.
 * \ingroup Core
 */
class T_DLLCLASS Event : public IWaitable
{
public:
	Event();

	virtual ~Event();
	
	void pulse(int count = 1);

	void broadcast();

	void reset();

	virtual bool wait(int32_t timeout = -1);
	
private:
#if defined(__PS3__)
	sys_lwmutex_t m_mutex;
	sys_lwcond_t m_cond;
	uint32_t m_signal;
	uint32_t m_waiters;
#elif defined(__PS4__)
	ScePthreadMutex m_mutex;
	ScePthreadCond m_cond;
	uint32_t m_signal;
	uint32_t m_waiters;
#else
	void* m_handle;
#endif
};

}

#endif	// traktor_Event_H
