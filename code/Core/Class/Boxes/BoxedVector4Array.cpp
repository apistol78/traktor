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
#include "Core/Class/Boxes/BoxedVector4Array.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedVector4Array, 16 > s_allocBoxedVector4Array;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4Array", BoxedVector4Array, Boxed)

void BoxedVector4Array::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedVector4Array::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedVector4Array::clear()
{
	m_arr.clear();
}

int32_t BoxedVector4Array::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedVector4Array::empty() const
{
	return m_arr.empty();
}

void BoxedVector4Array::push_back(const BoxedVector4* value)
{
	m_arr.push_back(value->unbox());
}

void BoxedVector4Array::pop_back()
{
	m_arr.pop_back();
}

Vector4 BoxedVector4Array::front()
{
	return m_arr.front();
}

Vector4 BoxedVector4Array::back()
{
	return m_arr.back();
}

void BoxedVector4Array::set(int32_t index, const BoxedVector4* value)
{
	m_arr[index] = value->unbox();
}

Vector4 BoxedVector4Array::get(int32_t index)
{
	return m_arr[index];
}

std::wstring BoxedVector4Array::toString() const
{
	return L"(Vector4 array)";
}

void* BoxedVector4Array::operator new (size_t size)
{
	return s_allocBoxedVector4Array.alloc();
}

void BoxedVector4Array::operator delete (void* ptr)
{
	s_allocBoxedVector4Array.free(ptr);
}

}
