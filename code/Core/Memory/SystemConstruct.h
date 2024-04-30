/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"

namespace traktor
{

template < typename T >
[[nodiscard]] T* allocConstruct()
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T();
}

template < typename T, typename A >
[[nodiscard]] T* allocConstruct(A a)
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T(a);
}

template < typename T, typename A, typename B, typename C >
[[nodiscard]] T* allocConstruct(A a, B b, C c)
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T(a, b, c);
}

template < typename T >
void freeDestruct(T* t)
{
	if (t)
	{
		t->~T();
		Alloc::freeAlign(t);
	}
}

}

