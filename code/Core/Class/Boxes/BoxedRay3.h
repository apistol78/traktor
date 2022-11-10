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
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Ray3.h"

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
class T_DLLCLASS BoxedRay3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRay3() = default;

	explicit BoxedRay3(const Ray3& value);

	const Vector4& origin() const { return m_value.origin; }

	const Vector4& direction() const { return m_value.direction; }

	float distance(const BoxedVector4* v) { return m_value.distance(v->unbox()); }

	Vector4 mul(float f) const { return m_value * Scalar(f); }

	const Ray3& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Ray3 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Ray3, false >
{
	static std::wstring typeName() {
		return L"traktor.Ray3";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRay3 >(value.getObjectUnsafe());
	}
	static Any set(const Ray3& value) {
		return Any::fromObject(new BoxedRay3(value));
	}
	static const Ray3& get(const Any& value) {
		return static_cast< BoxedRay3* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Ray3&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Ray3&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRay3 >(value.getObjectUnsafe());
	}
	static Any set(const Ray3& value) {
		return Any::fromObject(new BoxedRay3(value));
	}
	static const Ray3& get(const Any& value) {
		return static_cast< BoxedRay3* >(value.getObject())->unbox();
	}
};

}
