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
#include "Core/Class/Boxes/BoxedQuaternion.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedQuaternion, 4096 > s_allocBoxedQuaternion;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Quaternion", BoxedQuaternion, Boxed)

BoxedQuaternion::BoxedQuaternion(const Quaternion& value)
:	m_value(value)
{
}

BoxedQuaternion::BoxedQuaternion(float x, float y, float z, float w)
:	m_value(x, y, z, w)
{
}

BoxedQuaternion::BoxedQuaternion(const BoxedVector4* axis, float angle)
{
	m_value = Quaternion::fromAxisAngle(axis->unbox(), Scalar(angle));
}

BoxedQuaternion::BoxedQuaternion(float head, float pitch, float bank)
{
	m_value = Quaternion::fromEulerAngles(head, pitch, bank);
}

BoxedQuaternion::BoxedQuaternion(const BoxedVector4* from, const BoxedVector4* to)
:	m_value(from->unbox(), to->unbox())
{
}

BoxedQuaternion::BoxedQuaternion(const BoxedMatrix44* m)
:	m_value(m->unbox())
{
}

Quaternion BoxedQuaternion::normalized() const
{
	return m_value.normalized();
}

Quaternion BoxedQuaternion::inverse() const
{
	return m_value.inverse();
}

Quaternion BoxedQuaternion::concat(const BoxedQuaternion* q) const
{
	return m_value * q->m_value;
}

Vector4 BoxedQuaternion::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

Vector4 BoxedQuaternion::getEulerAngles() const
{
	return m_value.toEulerAngles();
}

Vector4 BoxedQuaternion::getAxisAngle() const
{
	return m_value.toAxisAngle();
}

Quaternion BoxedQuaternion::fromEulerAngles(float head, float pitch, float bank)
{
	return Quaternion::fromEulerAngles(head, pitch, bank);
}

Quaternion BoxedQuaternion::fromAxisAngle(const BoxedVector4* axisAngle)
{
	return Quaternion::fromAxisAngle(axisAngle->unbox());
}

std::wstring BoxedQuaternion::toString() const
{
	StringOutputStream ss;
	ss << m_value.e;
	return ss.str();
}

void* BoxedQuaternion::operator new (size_t size)
{
	return s_allocBoxedQuaternion.alloc();
}

void BoxedQuaternion::operator delete (void* ptr)
{
	s_allocBoxedQuaternion.free(ptr);
}

}
