/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Math/Bezier3rd.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedBezier2nd;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedBezier3rd : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedBezier3rd() = default;

	explicit BoxedBezier3rd(const Bezier3rd& value);

	explicit BoxedBezier3rd(const BoxedVector2* cp0, const BoxedVector2* cp1, const BoxedVector2* cp2, const BoxedVector2* cp3);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float flatness() const;

	bool isFlat(float tolerance) const;

	RefArray< BoxedBezier3rd > split(float t) const;

	RefArray< BoxedBezier2nd > approximate(float errorThreshold, int maxSubdivisions) const;

	const Vector2& cp0() const { return m_value.cp0; }

	const Vector2& cp1() const { return m_value.cp1; }

	const Vector2& cp2() const { return m_value.cp2; }

	const Vector2& cp3() const { return m_value.cp3; }

	const Bezier3rd& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

private:
	Bezier3rd m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Bezier3rd, false >
{
	static std::wstring typeName() {
		return L"traktor.Bezier3rd";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier3rd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier3rd& value) {
		return Any::fromObject(new BoxedBezier3rd(value));
	}
	static const Bezier3rd& get(const Any& value) {
		return static_cast< BoxedBezier3rd* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Bezier3rd&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Bezier3rd&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier3rd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier3rd& value) {
		return Any::fromObject(new BoxedBezier3rd(value));
	}
	static const Bezier3rd& get(const Any& value) {
		return static_cast< BoxedBezier3rd* >(value.getObject())->unbox();
	}
};

}
