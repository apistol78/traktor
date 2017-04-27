/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Semaphore_H
#define traktor_Semaphore_H

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

/*! \brief Blocking semaphore primitive.
 * \ingroup Core
 */
class T_DLLCLASS Semaphore : public IWaitable
{
public:
	Semaphore();
	
	virtual ~Semaphore();

	virtual bool wait(int32_t timeout = -1);
	
	void release();

private:
#if defined(__PS3__)
	sys_lwmutex_t m_mutex;
#elif defined(__PS4__)
	ScePthreadMutex m_mutex;
#else
	void* m_handle;
#endif
};

}

#endif	// traktor_Semaphore_H
