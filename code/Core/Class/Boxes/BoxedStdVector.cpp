/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedStdVector, 16 > s_allocBoxedStdVector;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.StdVector", BoxedStdVector, Boxed)

BoxedStdVector::BoxedStdVector(uint32_t size)
:	m_arr(size)
{
}

void BoxedStdVector::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedStdVector::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedStdVector::clear()
{
	m_arr.clear();
}

int32_t BoxedStdVector::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedStdVector::empty() const
{
	return m_arr.empty();
}

void BoxedStdVector::push_back(const Any& value)
{
	m_arr.push_back(value);
}

void BoxedStdVector::pop_back()
{
	m_arr.pop_back();
}

const Any& BoxedStdVector::front()
{
	return m_arr.front();
}

const Any& BoxedStdVector::back()
{
	return m_arr.back();
}

void BoxedStdVector::set(int32_t index, const Any& value)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = value;
}

Any BoxedStdVector::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return Any();
}

std::wstring BoxedStdVector::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedStdVector::operator new (size_t size)
{
	return s_allocBoxedStdVector.alloc();
}

void BoxedStdVector::operator delete (void* ptr)
{
	s_allocBoxedStdVector.free(ptr);
}

}
