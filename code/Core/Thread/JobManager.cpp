/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{

JobManager& JobManager::getInstance()
{
	static JobManager* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new JobManager();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());

		int32_t coreCount = OS::getInstance().getCPUCoreCount();
		if (coreCount >= 2)
		{
			// Reserve one core for main thread for good measure.
			coreCount -= 1;
		}

		s_instance->m_queue.create(
			coreCount,
			Thread::Normal
		);
	}
	return *s_instance;
}

void JobManager::destroy()
{
	m_queue.destroy();
	delete this;
}

}
