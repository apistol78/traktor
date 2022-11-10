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
#include "Core/Math/Matrix44.h"
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
class T_DLLCLASS BoxedMatrix44 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedMatrix44() = default;

	explicit BoxedMatrix44(const Matrix44& value);

	explicit BoxedMatrix44(const BoxedVector4* axisX, const BoxedVector4* axisY, const BoxedVector4* axisZ, const BoxedVector4* translation);

	Vector4 axisX() const;

	Vector4 axisY() const;

	Vector4 axisZ() const;

	Plane planeX() const;

	Plane planeY() const;

	Plane planeZ() const;

	Vector4 translation() const;

	Vector4 diagonal() const;

	bool isOrtho() const;

	float determinant() const;

	Matrix44 transpose() const;

	Matrix44 inverse() const;

	void setColumn(int c, const BoxedVector4* v);

	Vector4 getColumn(int c);

	void setRow(int r, const BoxedVector4* v);

	Vector4 getRow(int r);

	void set(int r, int c, float v);

	float get(int r, int c) const;

	Matrix44 concat(const BoxedMatrix44* t) const;

	Vector4 transform(const BoxedVector4* v) const;

	const Matrix44& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Matrix44 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Matrix44, false >
{
	static std::wstring typeName() {
		return L"traktor.Matrix44";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}
	static const Matrix44& get(const Any& value) {
		return static_cast< BoxedMatrix44* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Matrix44&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Matrix44&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}
	static const Matrix44& get(const Any& value) {
		return static_cast< BoxedMatrix44* >(value.getObject())->unbox();
	}
};

}
