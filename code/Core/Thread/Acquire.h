/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Acquire synchronization primitive.
 * \ingroup Core
 *
 * Helper class to acquire and release synchronization
 * primitive in scopes.
 */
template < typename T >
class Acquire
{
public:
	Acquire(const Acquire< T >&) = delete;

	T_FORCE_INLINE Acquire< T >(T& lock)
	:	m_lock(lock)
	{
		const bool result = m_lock.wait();
		T_FATAL_ASSERT(result == true);
	}

	T_FORCE_INLINE ~Acquire()
	{
		m_lock.release();
	}

private:
	T& m_lock;
};

/*! Release synchronization primitive.
 * \ingroup Core
 *
 * Helper class to acquire and release synchronization
 * primitive in scopes.
 */
template < typename T >
class Release
{
public:
	Release(const Release< T >&) = delete;

	T_FORCE_INLINE Release(T& lock)
	:	m_lock(lock)
	{
		m_lock.release();
	}

	T_FORCE_INLINE ~Release()
	{
		const bool result = m_lock.wait();
		T_FATAL_ASSERT(result == true);
	}

private:
	T& m_lock;
};

}

