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
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Math/Aabb2.h"

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
class T_DLLCLASS BoxedAabb2 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAabb2() = default;

	explicit BoxedAabb2(const Aabb2& value);

	explicit BoxedAabb2(const BoxedVector2* min, const BoxedVector2* max);

	const Vector2& min() const { return m_value.mn; }

	const Vector2& max() const { return m_value.mx; }

	bool inside(const BoxedVector2* pt) const { return m_value.inside(pt->unbox()); }

	void contain(const BoxedVector2* pt) { m_value.contain(pt->unbox()); }

	Vector2 getCenter() const { return m_value.getCenter(); }

	Vector2 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	const Aabb2& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Aabb2 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Aabb2, false >
{
	static std::wstring typeName() {
		return L"traktor.Aabb2";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb2 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb2& value) {
		return Any::fromObject(new BoxedAabb2(value));
	}
	static const Aabb2& get(const Any& value) {
		return static_cast< BoxedAabb2* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Aabb2&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Aabb2&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb2 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb2& value) {
		return Any::fromObject(new BoxedAabb2(value));
	}
	static const Aabb2& get(const Any& value) {
		return static_cast< BoxedAabb2* >(value.getObject())->unbox();
	}
};

}
