/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRandomGeometry.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRandomGeometry, 4 > s_allocBoxedRandomGeometry;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.RandomGeometry", BoxedRandomGeometry, BoxedRandom)

BoxedRandomGeometry::BoxedRandomGeometry(const RandomGeometry& value)
:	m_value(value)
{
}

BoxedRandomGeometry::BoxedRandomGeometry(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandomGeometry::toString() const
{
	return L"(random geometry)";
}

void* BoxedRandomGeometry::operator new (size_t size)
{
	return s_allocBoxedRandomGeometry.alloc();
}

void BoxedRandomGeometry::operator delete (void* ptr)
{
	s_allocBoxedRandomGeometry.free(ptr);
}

}
