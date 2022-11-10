/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Rotator.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.Rotator", Rotator, ISerializable)

Rotator::Rotator()
:	m_A(Vector4::zero())
,	m_B(Vector4::zero())
,	m_C(Vector4::zero())
{
}

Rotator::Rotator(
	const Vector4& A,
	const Vector4& B,
	const Vector4& C
)
:	m_A(A)
,	m_B(B)
,	m_C(C)
{
}

Rotator::Rotator(const Quaternion& q)
{
	float head, pitch, bank;
	q.toEulerAngles(head, pitch, bank);
	m_A = Vector4(0.0f, head, 0.0f);
	m_B = Vector4(pitch, 0.0f, 0.0f);
	m_C = Vector4(0.0f, 0.0f, bank);
}

Quaternion Rotator::toQuaternion() const
{
	return (Quaternion::fromAxisAngle(m_A) * Quaternion::fromAxisAngle(m_B) * Quaternion::fromAxisAngle(m_C)).normalized();
}

Rotator Rotator::operator + (const Rotator& rh) const
{
	return Rotator(
		m_A + rh.m_A,
		m_B + rh.m_B,
		m_C + rh.m_C
	);
}

Rotator& Rotator::operator += (const Rotator& rh)
{
	*this = *this + rh;
	return *this;
}

Rotator Rotator::operator * (const Scalar& rh) const
{
	return Rotator(
		m_A * rh,
		m_B * rh,
		m_C * rh
	);
}

Rotator& Rotator::operator *= (const Scalar& rh)
{
	*this = *this * rh;
	return *this;
}

Rotator Rotator::fromEulerXYZ(float x, float y, float z)
{
	const Vector4 rotations[] =
	{
		Vector4(x, 0.0f, 0.0f),
		Vector4(0.0f, y, 0.0f),
		Vector4(0.0f, 0.0f, z)
	};
	return Rotator(rotations[0], rotations[1], rotations[2]);
}

Rotator Rotator::fromEulerYXZ(float x, float y, float z)
{
	const Vector4 rotations[] =
	{
		Vector4(0.0f, y, 0.0f),
		Vector4(x, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, z)
	};
	return Rotator(rotations[0], rotations[1], rotations[2]);
}

void Rotator::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"A", m_A);
	s >> Member< Vector4 >(L"B", m_B);
	s >> Member< Vector4 >(L"C", m_C);
}

}
