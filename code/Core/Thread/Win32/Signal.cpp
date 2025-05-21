/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Signal.h"

#include "Core/System.h"

namespace traktor
{

Signal::Signal()
{
	m_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
}

Signal::~Signal()
{
	CloseHandle(m_handle);
}

void Signal::set()
{
	SetEvent(m_handle);
}

void Signal::reset()
{
	ResetEvent(m_handle);
}

bool Signal::wait(int32_t timeout)
{
	const bool ret = bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);
	return ret;
}

}
