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
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Range.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedBezier3rd;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedBezier2nd : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedBezier2nd() = default;

	explicit BoxedBezier2nd(const Bezier2nd& value);

	explicit BoxedBezier2nd(const BoxedVector2* cp0, const BoxedVector2* cp1, const BoxedVector2* cp2);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float getLocalMinMaxY() const;

	float getLocalMinMaxX() const;

	Range< float > intersectX(float y) const;

	Range< float > intersectY(float x) const;

	RefArray< BoxedBezier2nd > split(float t) const;

	Ref< BoxedBezier3rd > toBezier3rd() const;

	const Vector2& cp0() const { return m_value.cp0; }

	const Vector2& cp1() const { return m_value.cp1; }

	const Vector2& cp2() const { return m_value.cp2; }

	const Bezier2nd& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

private:
	Bezier2nd m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Bezier2nd, false >
{
	static std::wstring typeName() {
		return L"traktor.Bezier2nd";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier2nd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier2nd& value) {
		return Any::fromObject(new BoxedBezier2nd(value));
	}
	static const Bezier2nd& get(const Any& value) {
		return static_cast< BoxedBezier2nd* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Bezier2nd&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Bezier2nd&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier2nd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier2nd& value) {
		return Any::fromObject(new BoxedBezier2nd(value));
	}
	static const Bezier2nd& get(const Any& value) {
		return static_cast< BoxedBezier2nd* >(value.getObject())->unbox();
	}
};

}
