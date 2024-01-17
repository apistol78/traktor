/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/TicketLock.h"

namespace traktor
{

bool TicketLock::wait(int32_t timeout)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();

    const int32_t ticket = m_next++;
    while (m_serving != ticket)
        thread->yield();

	return true;
}

void TicketLock::release()
{
	m_serving++;
}

}
