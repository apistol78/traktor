/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <mutex>
#include "Core/Thread/CriticalSection.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	std::timed_mutex mtx;
};

	}

CriticalSection::CriticalSection()
{
	InternalData* data = new InternalData();
	m_handle = data;
}

CriticalSection::~CriticalSection()
{
	delete reinterpret_cast< InternalData* >(m_handle);
}

bool CriticalSection::wait(int32_t timeout)
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	if (timeout < 0)
	{
		data->mtx.lock();
		return true;
	}
	else if (timeout == 0)
	{
		if (data->mtx.try_lock())
			return true;
	}
	else
	{
		auto until = std::chrono::steady_clock::now();
		until += std::chrono::milliseconds(timeout);
		do
		{
			if (data->mtx.try_lock_until(until))
				return true;
		}
		while (until <= std::chrono::steady_clock::now());
	}
	return false;
}

void CriticalSection::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	data->mtx.unlock();
}

}
