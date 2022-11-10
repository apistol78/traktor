/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedGuid.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedGuid, 512 > s_allocBoxedGuid;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Guid", BoxedGuid, Boxed)

BoxedGuid::BoxedGuid(const Guid& value)
:	m_value(value)
{
}

BoxedGuid::BoxedGuid(const std::wstring& value)
:	m_value(value)
{
}

std::wstring BoxedGuid::toString() const
{
	return m_value.format();
}

void* BoxedGuid::operator new (size_t size)
{
	return s_allocBoxedGuid.alloc();
}

void BoxedGuid::operator delete (void* ptr)
{
	s_allocBoxedGuid.free(ptr);
}

}
