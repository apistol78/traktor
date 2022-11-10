/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
	m_handle = TlsAlloc();
}

ThreadLocal::~ThreadLocal()
{
	TlsFree((DWORD)m_handle);
}

void ThreadLocal::set(void* ptr)
{
	TlsSetValue((DWORD)m_handle, ptr);
}

void* ThreadLocal::get() const
{
	return TlsGetValue((DWORD)m_handle);
}

}
