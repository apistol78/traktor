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
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedAabb3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAabb3() = default;

	explicit BoxedAabb3(const Aabb3& value);

	explicit BoxedAabb3(const BoxedVector4* min, const BoxedVector4* max);

	const Vector4& min() const { return m_value.mn; }

	const Vector4& max() const { return m_value.mx; }

	bool inside(const BoxedVector4* pt) const { return m_value.inside(pt->unbox()); }

	Aabb3 transform(const BoxedTransform* tf) const { return m_value.transform(tf->unbox()); }

	void contain(const BoxedVector4* pt) { m_value.contain(pt->unbox()); }

	Aabb3 scale(const Scalar& factor) const { return m_value.scale(factor); }

	Aabb3 expand(const Scalar& margin) const { return m_value.expand(margin); }

	const Vector4& getMin() const { return m_value.mn; }

	const Vector4& getMax() const { return m_value.mx; }

	Vector4 getCenter() const { return m_value.getCenter(); }

	Vector4 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	bool overlap(const BoxedAabb3* aabb) const { return m_value.overlap(aabb->unbox()); }

	Any intersectRay(const BoxedVector4* origin, const BoxedVector4* direction) const;

	const Aabb3& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Aabb3 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Aabb3, false >
{
	static std::wstring typeName() {
		return L"traktor.Aabb3";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}
	static const Aabb3& get(const Any& value) {
		return static_cast< BoxedAabb3* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Aabb3&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Aabb3&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}
	static const Aabb3& get(const Any& value) {
		return static_cast< BoxedAabb3* >(value.getObject())->unbox();
	}
};

}
