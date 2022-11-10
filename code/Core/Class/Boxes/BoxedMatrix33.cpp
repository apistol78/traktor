/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedMatrix33.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedMatrix33, 256 > s_allocBoxedMatrix33;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Matrix33", BoxedMatrix33, Boxed)

BoxedMatrix33::BoxedMatrix33(const Matrix33& value)
:	m_value(value)
{
}

Vector4 BoxedMatrix33::diagonal() const
{
	return m_value.diagonal();
}

float BoxedMatrix33::determinant() const
{
	return m_value.determinant();
}

Matrix33 BoxedMatrix33::transpose() const
{
	return m_value.transpose();
}

Matrix33 BoxedMatrix33::inverse() const
{
	return m_value.inverse();
}

void BoxedMatrix33::set(int r, int c, float v)
{
	m_value.e[r][c] = v;
}

float BoxedMatrix33::get(int r, int c) const
{
	return m_value.e[r][c];
}

Matrix33 BoxedMatrix33::concat(const BoxedMatrix33* t) const
{
	return m_value * t->unbox();
}

Vector2 BoxedMatrix33::transform(const BoxedVector2* v) const
{
	return m_value * v->unbox();
}

Matrix33 BoxedMatrix33::translate(float x, float y)
{
	return traktor::translate(x, y);
}

Matrix33 BoxedMatrix33::scale(float x, float y)
{
	return traktor::scale(x, y);
}

Matrix33 BoxedMatrix33::rotate(float angle)
{
	return traktor::rotate(angle);
}

std::wstring BoxedMatrix33::toString() const
{
	StringOutputStream ss;
	ss << m_value.e11 << L", " << m_value.e12 << L", " << m_value.e13 << Endl;
	ss << m_value.e21 << L", " << m_value.e22 << L", " << m_value.e23 << Endl;
	ss << m_value.e31 << L", " << m_value.e32 << L", " << m_value.e33 << Endl;
	return ss.str();
}

void* BoxedMatrix33::operator new (size_t size)
{
	return s_allocBoxedMatrix33.alloc();
}

void BoxedMatrix33::operator delete (void* ptr)
{
	return s_allocBoxedMatrix33.free(ptr);
}

}
