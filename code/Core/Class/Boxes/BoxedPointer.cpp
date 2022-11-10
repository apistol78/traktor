/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedPointer.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedPointer, 32 > s_allocBoxedPointer;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Pointer", BoxedPointer, Boxed)

std::wstring BoxedPointer::toString() const
{
	return L"(pointer)";
}

void* BoxedPointer::operator new (size_t size)
{
	return s_allocBoxedPointer.alloc();
}

void BoxedPointer::operator delete (void* ptr)
{
	s_allocBoxedPointer.free(ptr);
}

}
