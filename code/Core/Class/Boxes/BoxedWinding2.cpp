/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Class/Boxes/BoxedWinding2.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedWinding2, 256 > s_allocBoxedWinding2;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Winding2", BoxedWinding2, Boxed)

BoxedWinding2::BoxedWinding2(const Winding2& value)
:	m_value(value)
{
}

void BoxedWinding2::clear()
{
	m_value.clear();
}

void BoxedWinding2::push(const BoxedVector2* pt)
{
	m_value.push(pt->unbox());
}

bool BoxedWinding2::inside(const BoxedVector2* pt) const
{
	return m_value.inside(pt->unbox());
}

Vector2 BoxedWinding2::closest(const BoxedVector2* pt) const
{
	return m_value.closest(pt->unbox());
}

void BoxedWinding2::reserve(uint32_t size)
{
	m_value.reserve(size);
}

void BoxedWinding2::resize(uint32_t size)
{
	m_value.resize(size);
}

uint32_t BoxedWinding2::size() const
{
	return m_value.size();
}

bool BoxedWinding2::empty() const
{
	return m_value.empty();
}

Vector2 BoxedWinding2::get(uint32_t index) const
{
	return m_value[index];
}

void BoxedWinding2::set(uint32_t index, const BoxedVector2* pt)
{
	m_value[index] = pt->unbox();
}

std::wstring BoxedWinding2::toString() const
{
	return L"(winding2)";
}

void* BoxedWinding2::operator new (size_t size)
{
	return s_allocBoxedWinding2.alloc();
}

void BoxedWinding2::operator delete (void* ptr)
{
	s_allocBoxedWinding2.free(ptr);
}

}
