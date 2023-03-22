/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Impl/Tasks/TaskUpdateSessionManager.h"
#include "Online/Provider/ISessionManagerProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskUpdateSessionManager", TaskUpdateSessionManager, ITask)

TaskUpdateSessionManager::TaskUpdateSessionManager(ISessionManagerProvider* provider)
:	m_provider(provider)
,	m_completed(false)
{
}

void TaskUpdateSessionManager::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	m_provider->update();
	m_completed = true;
}

}
