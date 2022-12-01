/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedPlane.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedPlane, 256 > s_allocBoxedPlane;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Plane", BoxedPlane, Boxed)

BoxedPlane::BoxedPlane(const Plane& value)
:	m_value(value)
{
}

BoxedPlane::BoxedPlane(const Vector4& normal, float distance)
:	m_value(normal, Scalar(distance))
{
}

BoxedPlane::BoxedPlane(const Vector4& normal, const Vector4& pointInPlane)
:	m_value(normal, pointInPlane)
{
}

BoxedPlane::BoxedPlane(const Vector4& a, const Vector4& b, const Vector4& c)
:	m_value(a, b, c)
{
}

BoxedPlane::BoxedPlane(float a, float b, float c, float d)
:	m_value(a, b, c, d)
{
}

Ref< BoxedVector4 > BoxedPlane::intersectRay(
	const Vector4& origin,
	const Vector4& direction
) const
{
	Vector4 result;
	Scalar k;

	if (m_value.intersectRay(origin, direction, k, result))
		return new BoxedVector4(result);
	else
		return nullptr;
}

Ref< BoxedVector4 > BoxedPlane::intersectSegment(
	const Vector4& a,
	const Vector4& b
) const
{
	Vector4 result;
	Scalar k;

	if (m_value.intersectSegment(a, b, k, &result))
		return new BoxedVector4(result);
	else
		return nullptr;
}

Ref< BoxedVector4 > BoxedPlane::uniqueIntersectionPoint(
	const Plane& a,
	const Plane& b,
	const Plane& c
)
{
	Vector4 result;
	if (Plane::uniqueIntersectionPoint(a, b, c, result))
		return new BoxedVector4(result);
	else
		return nullptr;
}

std::wstring BoxedPlane::toString() const
{
	StringOutputStream ss;
	ss << m_value.normal() << L", " << m_value.distance();
	return ss.str();
}

void* BoxedPlane::operator new (size_t size)
{
	return s_allocBoxedPlane.alloc();
}

void BoxedPlane::operator delete (void* ptr)
{
	s_allocBoxedPlane.free(ptr);
}

}
