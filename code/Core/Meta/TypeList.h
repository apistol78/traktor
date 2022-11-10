/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Meta/Traits.h"

namespace traktor
{

/*! \ingroup Core */
//@{

struct Empty {};

/*! Type list template. */
template <
	typename Type1 = Empty,
	typename Type2 = Empty,
	typename Type3 = Empty,
	typename Type4 = Empty,
	typename Type5 = Empty
>
struct TypeList
{
	typedef typename IsPointer< Type1 >::type_t head_t;
	typedef TypeList< Type2, Type3, Type4, Type5 > tail_t;

	enum { length = tail_t::length + 1 };
};

/*! Type list template, empty specialization. */
template <>
struct TypeList< Empty, Empty, Empty, Empty, Empty >
{
	enum { length = 0 };
};

/*! Type list length. */
template < typename TL >
struct Length
{
	enum { value = TL::length };
};

/*! Get type at index from type list. */
template <
	typename TL,
	int Index,
	int Depth = 0,
	bool Match = (Index == Depth),
	bool OutOfBounds = (TL::length == 0)
>
struct Get
{
	typedef typename Get< typename TL::tail_t, Index, Depth + 1 >::type_t type_t;
};

/*! Get type at index from type list, found index specialization. */
template <
	typename TL,
	int Index,
	int Depth,
	bool OutOfBounds
>
struct Get < TL, Index, Depth, true, OutOfBounds >
{
	typedef typename TL::head_t type_t;
};

/*! Get type at index from type list, out of bounds specialization. */
template <
	typename TL,
	int Index,
	int Depth,
	bool Match
>
struct Get < TL, Index, Depth, Match, true >
{
	// Out of bounds; will cause compile error.
};

//@}

}

