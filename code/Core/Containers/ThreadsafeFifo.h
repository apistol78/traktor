/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/SpinLock.h"

namespace traktor
{

/*!
 */
template < typename T >
class ThreadsafeFifo
{
public:
	void put(const T& item)
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		m_fifo.push_back(item);
	}

	bool get(T& item)
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		if (!m_fifo.empty())
		{
			item = m_fifo.back();
			m_fifo.pop_back();
			return true;
		}
		else
			return false;
	}

	void clear()
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		m_fifo.clear();
	}

private:
	SpinLock m_lock;
	AlignedVector< T > m_fifo;
};

}
