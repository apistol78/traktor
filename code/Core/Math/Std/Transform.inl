/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Transform.h"

namespace traktor
{

T_MATH_INLINE Transform::Transform()
:	m_translation(Vector4::zero())
,	m_rotation(Quaternion::identity())
{
}

T_MATH_INLINE Transform::Transform(const Transform& tf)
:	m_translation(tf.m_translation)
,	m_rotation(tf.m_rotation)
{
}

T_MATH_INLINE Transform::Transform(const Vector4& translation, const Quaternion& rotation)
:	m_translation(translation.xyz0())
,	m_rotation(rotation)
{
}

T_MATH_INLINE Transform::Transform(const Vector4& translation)
:	m_translation(translation.xyz0())
,	m_rotation(Quaternion::identity())
{
}

T_MATH_INLINE Transform::Transform(const Quaternion& rotation)
:	m_translation(Vector4::zero())
,	m_rotation(rotation)
{
}

T_MATH_INLINE Transform::Transform(const Matrix44& mx)
{
	m_translation = mx.translation().xyz0();
	m_rotation = Quaternion(mx);
}

T_MATH_INLINE const Transform& Transform::identity()
{
	const static Transform c_identity(Vector4::zero(), Quaternion::identity());
	return c_identity;
}

T_MATH_INLINE const Vector4& Transform::translation() const
{
	return m_translation;
}

T_MATH_INLINE const Quaternion& Transform::rotation() const
{
	return m_rotation;
}

T_MATH_INLINE Vector4 Transform::axisX() const
{
	return m_rotation * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
}

T_MATH_INLINE Vector4 Transform::axisY() const
{
	return m_rotation * Vector4(0.0f, 1.0f, 0.0f, 0.0f);
}

T_MATH_INLINE Vector4 Transform::axisZ() const
{
	return m_rotation * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
}

T_MATH_INLINE Transform Transform::inverse() const
{
	Quaternion rotationInv = m_rotation.inverse().normalized();
	return Transform(
		-(rotationInv * m_translation),
		rotationInv
	);
}

T_MATH_INLINE Matrix44 Transform::toMatrix44() const
{
	return translate(m_translation) * m_rotation.toMatrix44();
}

T_MATH_INLINE Transform& Transform::operator = (const Transform& tf)
{
	m_translation = tf.m_translation;
	m_rotation = tf.m_rotation;
	return *this;
}

T_MATH_INLINE bool Transform::operator == (const Transform& rh) const
{
	return m_translation == rh.m_translation && m_rotation == rh.m_rotation;
}

T_MATH_INLINE bool Transform::operator != (const Transform& rh) const
{
	return m_translation != rh.m_translation || m_rotation != rh.m_rotation;
}

T_MATH_INLINE T_DLLCLASS Transform operator * (const Transform& lh, const Transform& rh)
{
	return Transform(
		(lh.translation() + lh.rotation() * rh.translation()).xyz0(),
		(lh.rotation() * rh.rotation()).normalized()
	);
}

T_MATH_INLINE T_DLLCLASS Vector4 operator * (const Transform& tf, const Vector4& v)
{
	return tf.translation() * v.w() + tf.rotation() * v;
}

Transform lerp(const Transform& a, const Transform& b, const Scalar& c)
{
	return Transform(
		lerp(a.translation(), b.translation(), c).xyz0(),
		slerp(a.rotation(), b.rotation(), c).normalized()
	);
}

}
