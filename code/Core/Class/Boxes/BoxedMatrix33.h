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
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedVector2;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedMatrix33 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedMatrix33() = default;

	explicit BoxedMatrix33(const Matrix33& value);

	Vector4 diagonal() const;

	float determinant() const;

	Matrix33 transpose() const;

	Matrix33 inverse() const;

	void set(int r, int c, float v);

	float get(int r, int c) const;

	Matrix33 concat(const BoxedMatrix33* t) const;

	Vector2 transform(const BoxedVector2* v) const;

	static Matrix33 translate(float x, float y);

	static Matrix33 scale(float x, float y);

	static Matrix33 rotate(float angle);

	const Matrix33& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Matrix33 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Matrix33, false >
{
	static std::wstring typeName() {
		return L"traktor.Matrix33";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix33 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix33& value) {
		return Any::fromObject(new BoxedMatrix33(value));
	}
	static const Matrix33& get(const Any& value) {
		return static_cast< BoxedMatrix33* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Matrix33&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Matrix33&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix33 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix33& value) {
		return Any::fromObject(new BoxedMatrix33(value));
	}
	static const Matrix33& get(const Any& value) {
		return static_cast< BoxedMatrix33* >(value.getObject())->unbox();
	}
};

}
