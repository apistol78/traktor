/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRange.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRange, 256 > s_allocBoxedRange;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Range", BoxedRange, Boxed)

std::wstring BoxedRange::toString() const
{
	StringOutputStream ss;
	ss << m_min.getWideString() << L" - " << m_max.getWideString();
	return ss.str();
}

void* BoxedRange::operator new (size_t size)
{
	return s_allocBoxedRange.alloc();
}

void BoxedRange::operator delete (void* ptr)
{
	s_allocBoxedRange.free(ptr);
}

}
