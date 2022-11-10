/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedSphere.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedSphere, 64 > s_allocBoxedSphere;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Sphere", BoxedSphere, Boxed)

BoxedSphere::BoxedSphere(const Sphere& value)
:	m_value(value)
{
}

BoxedSphere::BoxedSphere(const Vector4& center, float radius)
:	m_value(center, Scalar(radius))
{
}

float BoxedSphere::intersectRay(
	const Vector4& origin,
	const Vector4& direction
) const
{
	Scalar k;
	if (m_value.intersectRay(origin, direction, k))
		return k;
	else
		return -1.0f;
}

std::wstring BoxedSphere::toString() const
{
	StringOutputStream ss;
	ss << m_value.center << L", " << m_value.radius;
	return ss.str();
}

void* BoxedSphere::operator new (size_t size)
{
	return s_allocBoxedSphere.alloc();
}

void BoxedSphere::operator delete (void* ptr)
{
	s_allocBoxedSphere.free(ptr);
}

}
