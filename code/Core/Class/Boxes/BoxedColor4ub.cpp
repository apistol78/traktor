/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedColor4ub.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedColor4ub, 16 > s_allocBoxedColor4ub;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Color4ub", BoxedColor4ub, Boxed)

BoxedColor4ub::BoxedColor4ub(const Color4ub& value)
:	m_value(value)
{
}

BoxedColor4ub::BoxedColor4ub(uint8_t red, uint8_t green, uint8_t blue)
:	m_value(red, green, blue, 255)
{
}

BoxedColor4ub::BoxedColor4ub(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
:	m_value(red, green, blue, alpha)
{
}

std::wstring BoxedColor4ub::formatRGB() const
{
	StringOutputStream ss;
	FormatHex(ss, m_value.r, 2);
	FormatHex(ss, m_value.g, 2);
	FormatHex(ss, m_value.b, 2);
	return ss.str();
}

std::wstring BoxedColor4ub::formatARGB() const
{
	StringOutputStream ss;
	FormatHex(ss, m_value.r, 2);
	FormatHex(ss, m_value.g, 2);
	FormatHex(ss, m_value.b, 2);
	FormatHex(ss, m_value.a, 2);
	return ss.str();
}

std::wstring BoxedColor4ub::toString() const
{
	StringOutputStream ss;
	ss << int32_t(m_value.r) << L", " << int32_t(m_value.g) << L", " << int32_t(m_value.b) << L", " << int32_t(m_value.a);
	return ss.str();
}

void* BoxedColor4ub::operator new (size_t size)
{
	return s_allocBoxedColor4ub.alloc();
}

void BoxedColor4ub::operator delete (void* ptr)
{
	s_allocBoxedColor4ub.free(ptr);
}

}
