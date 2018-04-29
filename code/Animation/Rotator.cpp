/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Rotator.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
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

Quaternion Rotator::toQuaternion() const
{
	return
		Quaternion::fromAxisAngle(m_A) *
		Quaternion::fromAxisAngle(m_B) *
		Quaternion::fromAxisAngle(m_C);
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

Rotator Rotator::fromHeadPitchBank(float head, float pitch, float bank)
{
	const Vector4 rotations[] =
	{
		Vector4(0.0f, head, 0.0f),
		Vector4(pitch, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, bank)		
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
}
