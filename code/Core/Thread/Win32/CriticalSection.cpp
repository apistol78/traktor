/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System.h"
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	m_handle = new CRITICAL_SECTION;
	InitializeCriticalSection((LPCRITICAL_SECTION)m_handle);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_handle);
	delete static_cast< CRITICAL_SECTION* >(m_handle);
}

bool CriticalSection::wait(int32_t timeout)
{
	T_ASSERT_M (timeout == -1, L"Invalid timeout");
	EnterCriticalSection((LPCRITICAL_SECTION)m_handle);
	return true;
}

void CriticalSection::release()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)m_handle);
}

}
