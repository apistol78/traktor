/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Atomic.h"

namespace traktor
{

/*! Invoke method on pointer to object once then set pointer to null.
 * \ingroup Core
 */
// \{

template < typename Type, typename P0 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0), P0 p0)
{
	Type* ptr = Atomic::exchange< Type* >(ref, nullptr);
	if (ptr)
		(ptr->*M)(p0);
}

template < typename Type, typename P0, typename P1 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0, P1 p1), P0 p0, P1 p1)
{
	Type* ptr = Atomic::exchange< Type* >(ref, nullptr);
	if (ptr)
		(ptr->*M)(p0, p1);
}

template < typename Type, typename P0, typename P1, typename P2 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0, P1 p1, P2 p2), P0 p0, P1 p1, P2 p2)
{
	Type* ptr = Atomic::exchange< Type* >(ref, nullptr);
	if (ptr)
		(ptr->*M)(p0, p1, p2);
}

// \}

}

