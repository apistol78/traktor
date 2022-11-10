/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstddef>

namespace traktor
{

/*! Align down value.
 * \ingroup Core
 */
template < typename T >
T alignDown(T v, size_t a)
{
	return (T)((size_t)v & ~(a - 1));
}

/*! Align up value.
 * \ingroup Core.
 */
template < typename T >
T alignUp(T v, size_t a)
{
	return (T)(((size_t)v + a - 1) - ((size_t)v + a - 1) % a);
}

/*! Determine alignment of type.
 * \ingroup Core
 */
template < typename T >
size_t alignOf()
{
#if !defined(alignof)
#	if defined(_MSC_VER)
	return __alignof(T);
#	elif defined(__GNUC__)
	return __alignof__(T);
#	else
	struct __align_struct__ { char dummy1; T dummy2; };
	return offsetof(__align_struct, dummy2);
#	endif
#endif
}

}

