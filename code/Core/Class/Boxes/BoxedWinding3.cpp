/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/Boxes/BoxedWinding3.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedWinding3, 356 > s_allocBoxedWinding3;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Winding3", BoxedWinding3, Boxed)

BoxedWinding3::BoxedWinding3(const Winding3& value)
:	m_value(value)
{
}

void BoxedWinding3::clear()
{
	m_value.clear();
}

void BoxedWinding3::push(const BoxedVector4* pt)
{
	m_value.push(pt->unbox());
}

void BoxedWinding3::reserve(uint32_t size)
{
	m_value.reserve(size);
}

void BoxedWinding3::resize(uint32_t size)
{
	m_value.resize(size);
}

uint32_t BoxedWinding3::size() const
{
	return m_value.size();
}

bool BoxedWinding3::empty() const
{
	return m_value.empty();
}

Vector4 BoxedWinding3::get(uint32_t index) const
{
	return m_value[index];
}

void BoxedWinding3::set(uint32_t index, const BoxedVector4* pt)
{
	m_value[index] = pt->unbox();
}

std::wstring BoxedWinding3::toString() const
{
	return L"(winding3)";
}

void* BoxedWinding3::operator new (size_t size)
{
	return s_allocBoxedWinding3.alloc();
}

void BoxedWinding3::operator delete (void* ptr)
{
	s_allocBoxedWinding3.free(ptr);
}

}
