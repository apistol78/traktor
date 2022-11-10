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

/*! Thread atomic operations.
 * \ingroup Core
 *
 * Perform atomic operations on variables.
 */
struct T_DLLCLASS Atomic
{
private:
	template < typename T, int Tsize >
	struct S
	{
	};

	template < typename T > struct S < T, 4 >
	{
		inline static T exchange(T& s, T v) { return (T)Atomic::exchange(*(uint32_t*)&s, *(uint32_t*)&v); }
	};

	template < typename T > struct S < T, 8 >
	{
		inline static T exchange(T& s, T v) { return (T)Atomic::exchange(*(uint64_t*)&s, *(uint64_t*)&v); }
	};

public:
	/*! Increment variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int32_t increment(int32_t& value);

	/*! Decrement variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int32_t decrement(int32_t& value);

	/*! Add variable.
	 *
	 * \return Initial value of variable.
	 */
	static T_ATOMIC_INLINE int32_t add(int32_t& value, int32_t delta);

	/*! Add variable.
	 *
	 * \return Result value of variable.
	 */
	static T_ATOMIC_INLINE int64_t add(int64_t& value, int64_t delta);

	/*! Set value of variable. */
	static T_ATOMIC_INLINE uint32_t exchange(uint32_t& s, uint32_t v);

	/*! Set value of variable. */
	static T_ATOMIC_INLINE uint64_t exchange(uint64_t& s, uint64_t v);

	/*! Compare and swap. */
	static T_ATOMIC_INLINE int32_t compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual);

	/*! Set value of variable. */
	template < typename T >
	static T exchange(T& s, T v)
	{
		return S< T, sizeof(T) >::exchange(s, v);
	}
};

}

#if !defined(_DEBUG)
#	if defined(_WIN32)
#		include "Core/Thread/Win32/Atomic.inl"
#	elif defined(__APPLE__)
#		include "Core/Thread/OsX/Atomic.inl"
#	else
#		include "Core/Thread/Linux/Atomic.inl"
#	endif
#endif
