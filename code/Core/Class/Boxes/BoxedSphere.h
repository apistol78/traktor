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
#include "Core/Math/Sphere.h"

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
class T_DLLCLASS BoxedSphere : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedSphere() = default;

	explicit BoxedSphere(const Sphere& value);

	explicit BoxedSphere(const Vector4& center, float radius);

	void setCenter(const Vector4& center) { m_value.center = center; }

	void setRadius(float radius) { m_value.radius = Scalar(radius); }

	const Vector4& center() const { return m_value.center; }

	float radius() const { return m_value.radius; }

	bool inside(const Vector4& pt) const { return m_value.inside(pt); }

	float intersectRay(const Vector4& origin, const Vector4& direction) const;

	const Sphere& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Sphere m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Sphere, false >
{
	static std::wstring typeName() {
		return L"traktor.Sphere";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedSphere >(value.getObjectUnsafe());
	}
	static Any set(const Sphere& value) {
		return Any::fromObject(new BoxedSphere(value));
	}
	static const Sphere& get(const Any& value) {
		return static_cast< BoxedSphere* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Sphere&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Sphere&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedSphere >(value.getObjectUnsafe());
	}
	static Any set(const Sphere& value) {
		return Any::fromObject(new BoxedSphere(value));
	}
	static const Sphere& get(const Any& value) {
		return static_cast< BoxedSphere* >(value.getObject())->unbox();
	}
};

}
