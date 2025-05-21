/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Mutex.h"

#include "Core/Misc/TString.h"
#include "Core/System.h"

namespace traktor
{

Mutex::Mutex()
	: m_existing(false)
{
	m_handle = CreateMutex(0, FALSE, NULL);
	T_ASSERT(m_handle != NULL);
}

Mutex::Mutex(const Guid& id)
{
	m_handle = CreateMutex(
		0,
		FALSE,
		wstots(L"Global\\" + id.format()).c_str());
	m_existing = bool(GetLastError() == ERROR_ALREADY_EXISTS);
	T_ASSERT(m_handle != NULL);
}

Mutex::~Mutex()
{
	CloseHandle(m_handle);
}

bool Mutex::wait(int32_t timeout)
{
	const bool ret = bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);
	return ret;
}

void Mutex::release()
{
	ReleaseMutex(m_handle);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
