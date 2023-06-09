/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Containers/AlignedVector.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/CriticalSection.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

/*! OS thread manager.
 * \ingroup Core
 *
 * All threads should be created and destroyed
 * through the ThreadManager.
 */
class T_DLLCLASS ThreadManager : public ISingleton
{
public:
	typedef std::function< void() > threadFn_t;

	static ThreadManager& getInstance();

	/*! Create thread.
	 *
	 * Create a thread from a functor.
	 * An optional name can be assigned to the thread
	 * which is presented in the debugger.
	 *
	 * \param functor Functor object which will be called when the thread starts.
	 * \param name Debug name of thread.
	 * \param hardwareCore Preferred hardware core, -1 = any core.
	 * \return Thread object.
	 */
	Thread* create(const threadFn_t& fn, const wchar_t* const name = L"Unnamed", int hardwareCore = -1);

	/*! Destroy thread.
	 *
	 * Destroy a thread.
	 * All threads must be destroyed through this
	 * method, otherwise the application will assert in debug or
	 * leak resources in release.
	 *
	 * \param thread Thread object to destroy.
	 */
	void destroy(Thread* thread);

	/*! Get current thread.
	 *
	 * Get thread object of currently executing thread.
	 *
	 * \return Thread object.
	 */
	Thread* getCurrentThread();

	/*! Get main thread.
	 * 
	 * \return Thread object.
	 */
	Thread* getMainThread();

protected:
	virtual void destroy();

private:
	CriticalSection m_threadsLock;
	AlignedVector< Thread* > m_threads;
	Thread* m_threadBase;

	ThreadManager();

	~ThreadManager();
};

}
