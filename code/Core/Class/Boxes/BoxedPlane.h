/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Plane.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedVector4;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedPlane : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedPlane() = default;

	explicit BoxedPlane(const Plane& value);

	explicit BoxedPlane(const Vector4& normal, float distance);

	explicit BoxedPlane(const Vector4& normal, const Vector4& pointInPlane);

	explicit BoxedPlane(const Vector4& a, const Vector4& b, const Vector4& c);

	explicit BoxedPlane(float a, float b, float c, float d);

	void setNormal(const Vector4& normal) { m_value.setNormal(normal); }

	void setDistance(float distance) { m_value.setDistance(Scalar(distance)); }

	Vector4 normal() const { return m_value.normal(); }

	float distance() const { return m_value.distance(); }

	float distanceToPoint(const Vector4& point) const { return m_value.distance(point); }

	Vector4 project(const Vector4& v) const { return m_value.project(v); }

	Ref< BoxedVector4 > intersectRay(
		const Vector4& origin,
		const Vector4& direction
	) const;

	Ref< BoxedVector4 > intersectSegment(
		const Vector4& a,
		const Vector4& b
	) const;

	static Ref< BoxedVector4 > uniqueIntersectionPoint(
		const Plane& a,
		const Plane& b,
		const Plane& c
	);

	const Plane& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Plane m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Plane, false >
{
	static std::wstring typeName() {
		return L"traktor.Plane";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static const Plane& get(const Any& value) {
		return static_cast< BoxedPlane* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Plane&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Plane&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static const Plane& get(const Any& value) {
		return static_cast< BoxedPlane* >(value.getObject())->unbox();
	}
};

}
