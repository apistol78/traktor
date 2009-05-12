#ifndef traktor_Acquire_H
#define traktor_Acquire_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Acquire synchronization primitive.
 * \ingroup Core
 *
 * Helper class to acquire and release synchronization
 * primitive in scopes.
 */
template < typename T >
class Acquire
{
public:
	T_FORCE_INLINE Acquire< T >(T& lock)
	:	m_lock(lock)
	{
		m_lock.acquire();
	}
	
	T_FORCE_INLINE ~Acquire< T >()
	{
		m_lock.release();
	}

private:
	T& m_lock;
};

/*! \brief Release synchronization primitive.
 * \ingroup Core
 *
 * Helper class to acquire and release synchronization
 * primitive in scopes.
 */
template < typename T >
class Release
{
public:
	T_FORCE_INLINE Release< T >(T& lock)
	:	m_lock(lock)
	{
		m_lock.release();
	}
	
	T_FORCE_INLINE ~Release< T >()
	{
		m_lock.acquire();
	}

private:
	T& m_lock;
};

}

#endif	// traktor_Acquire_H
