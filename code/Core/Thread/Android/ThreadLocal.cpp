/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <pthread.h>
#include "Core/Platform.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	int32_t rc = pthread_key_create(&key, 0);
	T_FATAL_ASSERT_M(rc == 0, L"Unable to create thread key");
}

ThreadLocal::~ThreadLocal()
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_key_delete(key);
}

void ThreadLocal::set(void* ptr)
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_setspecific(key, ptr);
}

void* ThreadLocal::get() const
{
	const pthread_key_t& key = *reinterpret_cast< const pthread_key_t* >(&m_handle);
	return pthread_getspecific(key);
}

}
