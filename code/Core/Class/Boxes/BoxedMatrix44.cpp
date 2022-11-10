/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedMatrix44.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedMatrix44, 16 > s_allocBoxedMatrix44;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Matrix44", BoxedMatrix44, Boxed)

BoxedMatrix44::BoxedMatrix44(const Matrix44& value)
:	m_value(value)
{
}

BoxedMatrix44::BoxedMatrix44(const BoxedVector4* axisX, const BoxedVector4* axisY, const BoxedVector4* axisZ, const BoxedVector4* translation)
:	m_value(axisX->unbox(), axisY->unbox(), axisZ->unbox(), translation->unbox())
{
}

Vector4 BoxedMatrix44::axisX() const
{
	return m_value.axisX();
}

Vector4 BoxedMatrix44::axisY() const
{
	return m_value.axisY();
}

Vector4 BoxedMatrix44::axisZ() const
{
	return m_value.axisZ();
}

Plane BoxedMatrix44::planeX() const
{
	return Plane(m_value.axisX(), m_value.translation());
}

Plane BoxedMatrix44::planeY() const
{
	return Plane(m_value.axisY(), m_value.translation());
}

Plane BoxedMatrix44::planeZ() const
{
	return Plane(m_value.axisZ(), m_value.translation());
}

Vector4 BoxedMatrix44::translation() const
{
	return m_value.translation();
}

Vector4 BoxedMatrix44::diagonal() const
{
	return m_value.diagonal();
}

bool BoxedMatrix44::isOrtho() const
{
	return m_value.isOrtho();
}

float BoxedMatrix44::determinant() const
{
	return m_value.determinant();
}

Matrix44 BoxedMatrix44::transpose() const
{
	return m_value.transpose();
}

Matrix44 BoxedMatrix44::inverse() const
{
	return m_value.inverse();
}

void BoxedMatrix44::setColumn(int c, const BoxedVector4* v)
{
	m_value.set(c, v->unbox());
}

Vector4 BoxedMatrix44::getColumn(int c)
{
	return m_value.get(c);
}

void BoxedMatrix44::setRow(int r, const BoxedVector4* v)
{
	const Vector4& vv = v->unbox();
	m_value.set(r, 0, vv.x());
	m_value.set(r, 1, vv.x());
	m_value.set(r, 2, vv.x());
	m_value.set(r, 3, vv.x());
}

Vector4 BoxedMatrix44::getRow(int r)
{
	return Vector4(
		m_value.get(r, 0),
		m_value.get(r, 1),
		m_value.get(r, 2),
		m_value.get(r, 3)
	);
}

void BoxedMatrix44::set(int r, int c, float v)
{
	m_value.set(r, c, Scalar(v));
}

float BoxedMatrix44::get(int r, int c) const
{
	return m_value.get(r, c);
}

Matrix44 BoxedMatrix44::concat(const BoxedMatrix44* t) const
{
	return m_value * t->m_value;
}

Vector4 BoxedMatrix44::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

std::wstring BoxedMatrix44::toString() const
{
	StringOutputStream ss;
	ss << m_value(0, 0) << L", " << m_value(0, 1) << L", " << m_value(0, 2) << L", " << m_value(0, 3) << Endl;
	ss << m_value(1, 0) << L", " << m_value(1, 1) << L", " << m_value(1, 2) << L", " << m_value(1, 3) << Endl;
	ss << m_value(2, 0) << L", " << m_value(2, 1) << L", " << m_value(2, 2) << L", " << m_value(2, 3) << Endl;
	ss << m_value(3, 0) << L", " << m_value(3, 1) << L", " << m_value(3, 2) << L", " << m_value(3, 3) << Endl;
	return ss.str();
}

void* BoxedMatrix44::operator new (size_t size)
{
	return s_allocBoxedMatrix44.alloc();
}

void BoxedMatrix44::operator delete (void* ptr)
{
	s_allocBoxedMatrix44.free(ptr);
}

}
