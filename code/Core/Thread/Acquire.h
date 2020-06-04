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
	Acquire< T >(const Acquire< T >&) = delete;

	T_FORCE_INLINE Acquire< T >(T& lock)
	:	m_lock(lock)
	{
		m_lock.wait();
	}

	T_FORCE_INLINE ~Acquire< T >()
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
	Release< T >(const Release< T >&) = delete;

	T_FORCE_INLINE Release< T >(T& lock)
	:	m_lock(lock)
	{
		m_lock.release();
	}

	T_FORCE_INLINE ~Release< T >()
	{
		m_lock.wait();
	}

private:
	T& m_lock;
};

}

