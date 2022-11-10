/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRefArray, 512 > s_allocBoxedRefArray;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.RefArray", BoxedRefArray, Boxed)

void BoxedRefArray::push_back(ITypedObject* object)
{
	m_arr.push_back(object);
}

void BoxedRefArray::pop_back()
{
	m_arr.pop_back();
}

bool BoxedRefArray::remove(ITypedObject* object)
{
	return m_arr.remove(object);
}

std::wstring BoxedRefArray::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedRefArray::operator new (size_t size)
{
	return s_allocBoxedRefArray.alloc();
}

void BoxedRefArray::operator delete (void* ptr)
{
	s_allocBoxedRefArray.free(ptr);
}

}
