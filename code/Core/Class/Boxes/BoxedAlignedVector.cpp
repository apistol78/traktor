/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedAlignedVector, 16 > s_allocBoxedAlignedVector;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.AlignedVector", BoxedAlignedVector, Boxed)

BoxedAlignedVector::BoxedAlignedVector(uint32_t size)
:	m_arr(size)
{
}

void BoxedAlignedVector::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedAlignedVector::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedAlignedVector::clear()
{
	m_arr.clear();
}

int32_t BoxedAlignedVector::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedAlignedVector::empty() const
{
	return m_arr.empty();
}

void BoxedAlignedVector::push_back(const Any& value)
{
	m_arr.push_back(value);
}

void BoxedAlignedVector::pop_back()
{
	m_arr.pop_back();
}

const Any& BoxedAlignedVector::front()
{
	return m_arr.front();
}

const Any& BoxedAlignedVector::back()
{
	return m_arr.back();
}

void BoxedAlignedVector::set(int32_t index, const Any& value)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = value;
}

Any BoxedAlignedVector::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return Any();
}

std::wstring BoxedAlignedVector::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedAlignedVector::operator new (size_t size)
{
	return s_allocBoxedAlignedVector.alloc();
}

void BoxedAlignedVector::operator delete (void* ptr)
{
	s_allocBoxedAlignedVector.free(ptr);
}

}
