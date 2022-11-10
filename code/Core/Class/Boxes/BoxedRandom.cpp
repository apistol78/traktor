/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRandom.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRandom, 4 > s_allocBoxedRandom;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Random", BoxedRandom, Boxed)

BoxedRandom::BoxedRandom(const Random& value)
:	m_value(value)
{
}

BoxedRandom::BoxedRandom(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandom::toString() const
{
	return L"(random)";
}

void* BoxedRandom::operator new (size_t size)
{
	return s_allocBoxedRandom.alloc();
}

void BoxedRandom::operator delete (void* ptr)
{
	s_allocBoxedRandom.free(ptr);
}

}
