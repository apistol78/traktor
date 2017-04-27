/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Atomic_H
#define traktor_Atomic_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

#if !defined(_DEBUG)
#	define T_ATOMIC_INLINE T_FORCE_INLINE
#else
#	define T_ATOMIC_INLINE
#endif

/*! \brief Thread atomic operations.
 * \ingroup Core
 *
 * Perform atomic operations on variables.
 */
struct T_DLLCLASS Atomic
{
	/*! \brief Increment variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int32_t increment(int32_t& value);

	/*! \brief Decrement variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int32_t decrement(int32_t& value);

	/*! \brief Add variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int32_t add(int32_t& value, int32_t delta);

	/*! \brief Set value of variable. */
	static T_ATOMIC_INLINE uint32_t exchange(uint32_t& s, uint32_t v);

	/*! \brief Set value of variable. */
	static T_ATOMIC_INLINE uint64_t exchange(uint64_t& s, uint64_t v);

	/*! \brief Compare and swap. */
	static T_ATOMIC_INLINE int32_t compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual);

	/*! \brief Set value of variable. */
	template < typename T >
	static T exchange(T& s, T v)
	{
		if (sizeof(T) <= sizeof(uint32_t))
			return (T)exchange(*(uint32_t*)&s, *(uint32_t*)&v);
		else
			return (T)exchange(*(uint64_t*)&s, *(uint64_t*)&v);
	}
};

}

#if !defined(_DEBUG)
#	if defined(_WIN32)
#		include "Core/Thread/Win32/Atomic.inl"
#	elif defined(__APPLE__)
#		include "Core/Thread/OsX/Atomic.inl"
#	elif defined(_PS3)
#		include "Core/Thread/Ps3/Atomic.inl"
#	else
#		include "Core/Thread/Linux/Atomic.inl"
#	endif
#endif

#endif	// traktor_Atomic_H
