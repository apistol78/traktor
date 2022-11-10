/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Quaternion.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/* Rotator
 * \ingroup Core
 *
 * A rotator is allows for more than 360 degrees of rotation
 * with arbitrary axis order.
 *
 * Rotation is defined using three axis-angle rotations.
 */
class T_DLLCLASS Rotator : public ISerializable
{
	T_RTTI_CLASS;

public:
	Rotator();

	explicit Rotator(
		const Vector4& A,
		const Vector4& B,
		const Vector4& C
	);

	explicit Rotator(const Quaternion& q);

	Quaternion toQuaternion() const;

	Rotator operator + (const Rotator& rh) const;

	Rotator& operator += (const Rotator& rh);

	Rotator operator * (const Scalar& rh) const;

	Rotator& operator *= (const Scalar& rh);

	static Rotator fromEulerXYZ(float x, float y, float z);

	static Rotator fromEulerYXZ(float x, float y, float z);

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_A;
	Vector4 m_B;
	Vector4 m_C;
};

inline Rotator lerp(const Rotator& a, const Rotator& b, const Scalar& c)
{
	return a * (Scalar(1.0f) - c) + b * c;
}

}
