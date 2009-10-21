#ifndef traktor_Quaternion_H
#define traktor_Quaternion_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Quaternion
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Quaternion
{
public:
	union
	{
		float e[4];
		struct { float x, y, z, w; };
	};

	T_MATH_INLINE Quaternion();

	T_MATH_INLINE Quaternion(const Quaternion& q);

	explicit T_MATH_INLINE Quaternion(float x, float y, float z, float w);

	explicit T_MATH_INLINE Quaternion(const Vector4& axisAngle);

	explicit T_MATH_INLINE Quaternion(const Matrix44& matrixOrientation);

	explicit T_MATH_INLINE Quaternion(float head, float pitch, float bank);

	explicit T_MATH_INLINE Quaternion(const Vector4& from, const Vector4& to);

	static T_MATH_INLINE Quaternion identity();

	T_MATH_INLINE Quaternion normalized() const;

	T_MATH_INLINE Quaternion inverse() const;

	T_MATH_INLINE Vector4 toAxisAngle() const;

	T_MATH_INLINE Matrix44 toMatrix44() const;

	T_MATH_INLINE void toEulerAngles(float& outHead, float& outPitch, float& outBank) const;

	T_MATH_INLINE Quaternion& operator *= (const Quaternion& r);

	T_MATH_INLINE bool operator == (const Quaternion& v) const;

	T_MATH_INLINE bool operator != (const Quaternion& v) const;

	friend T_MATH_INLINE T_DLLCLASS Quaternion operator + (const Quaternion& l, const Quaternion& r);

	friend T_MATH_INLINE T_DLLCLASS Quaternion operator * (const Quaternion& l, float r);

	friend T_MATH_INLINE T_DLLCLASS Quaternion operator * (const Quaternion& l, const Quaternion& r);

	friend T_MATH_INLINE T_DLLCLASS Vector4 operator * (const Quaternion& q, const Vector4& v);
};

T_MATH_INLINE T_DLLCLASS Quaternion lerp(const Quaternion& a, const Quaternion& b, float c);

T_MATH_INLINE T_DLLCLASS Quaternion slerp(const Quaternion& a, const Quaternion& b, float c);

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Quaternion.inl"
#endif

#endif	// traktor_Quaternion_H
