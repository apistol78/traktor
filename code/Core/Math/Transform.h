#ifndef traktor_Transform_H
#define traktor_Transform_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

/*! \brief Linear transform.
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Transform
{
public:
	T_MATH_INLINE Transform();

	T_MATH_INLINE Transform(const Transform& tf);

	explicit T_MATH_INLINE Transform(const Vector4& translation, const Quaternion& rotation);

	explicit T_MATH_INLINE Transform(const Vector4& translation);

	explicit T_MATH_INLINE Transform(const Quaternion& rotation);

	explicit T_MATH_INLINE Transform(const Matrix44& mx);

	static T_MATH_INLINE const Transform& identity();

	T_MATH_INLINE const Vector4& translation() const;

	T_MATH_INLINE const Quaternion& rotation() const;

	T_MATH_INLINE Vector4 axisX() const;

	T_MATH_INLINE Vector4 axisY() const;
	
	T_MATH_INLINE Vector4 axisZ() const;

	T_MATH_INLINE Transform inverse() const;

	T_MATH_INLINE Matrix44 toMatrix44() const;

	T_MATH_INLINE Transform& operator = (const Transform& tf);

	T_MATH_INLINE bool operator == (const Transform& rh) const;

	T_MATH_INLINE bool operator != (const Transform& rh) const;

#if !(defined(_PS3) && defined(SPU))
	void serialize(ISerializer& s);
#endif

	friend T_MATH_INLINE T_DLLCLASS Transform operator * (const Transform& rh, const Transform& lh);

	friend T_MATH_INLINE T_DLLCLASS Vector4 operator * (const Transform& tf, const Vector4& v);

private:
	Vector4 m_translation;
	Quaternion m_rotation;
};

T_MATH_INLINE T_DLLCLASS Transform lerp(const Transform& a, const Transform& b, const Scalar& c);

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Transform.inl"
#endif

#endif	// traktor_Transform_H
