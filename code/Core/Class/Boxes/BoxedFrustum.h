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
#include "Core/Math/Frustum.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedAabb3;
class BoxedVector4;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedFrustum : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedFrustum() = default;

	explicit BoxedFrustum(const Frustum& value);

	void buildPerspective(float vfov, float aspect, float zn, float zf);

	void buildOrtho(float width, float height, float zn, float zf);

	void setNearZ(float zn);

	float getNearZ() const;

	void setFarZ(float zf);

	float getFarZ() const;

	bool insidePoint(const BoxedVector4* point) const;

	int32_t insideSphere(const BoxedVector4* center, float radius) const;

	int32_t insideAabb(const BoxedAabb3* aabb) const;

	const Plane& getPlane(int32_t index) const;

	const Vector4& getCorner(int32_t index) const;

	const Vector4& getCenter() const;

	const Frustum& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Frustum m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Frustum, false >
{
	static std::wstring typeName() {
		return L"traktor.Frustum";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}
	static const Frustum& get(const Any& value) {
		return static_cast< BoxedFrustum* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Frustum&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Frustum&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}
	static const Frustum& get(const Any& value) {
		return static_cast< BoxedFrustum* >(value.getObject())->unbox();
	}
};

}
