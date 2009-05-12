#ifndef traktor_Matrix44_H
#define traktor_Matrix44_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief 4x4 matrix.
 * \ingroup Core
 *
 *	[x-axis   ]
 *	[y-axis   ]
 *	[z-axis   ]
 *	[translate]
 *
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 * 12 13 14 15
 */
T_MATH_ALIGN16 class T_DLLCLASS Matrix44
{
public:
	union
	{
		union { float m[16]; };			// m[row * 4 + column]
		union { float e[4][4]; };		// e[row][colum]
		struct
		{
			float e11, e12, e13, e14;	// named as e[row][column]
			float e21, e22, e23, e24;
			float e31, e32, e33, e34;
			float e41, e42, e43, e44;
		};
	};

	T_MATH_INLINE Matrix44();

	T_MATH_INLINE Matrix44(const Matrix44& m);

	explicit T_MATH_INLINE Matrix44(const Vector4& axisX, const Vector4& axisY, const Vector4& axisZ, const Vector4& translation);

	explicit T_MATH_INLINE Matrix44(float e11, float e12, float e13, float e14, float e21, float e22, float e23, float e24, float e31, float e32, float e33, float e34, float e41, float e42, float e43, float e44);

	explicit T_MATH_INLINE Matrix44(const float* p);

	static T_MATH_INLINE const Matrix44& zero();

	static T_MATH_INLINE const Matrix44& identity();

	T_MATH_INLINE Vector4 axisX() const;

	T_MATH_INLINE Vector4 axisY() const;

	T_MATH_INLINE Vector4 axisZ() const;

	T_MATH_INLINE Vector4 translation() const;

	T_MATH_INLINE Vector4 diagonal() const;

	T_MATH_INLINE bool isOrtho() const;

	T_MATH_INLINE float determinant() const;

	T_MATH_INLINE Matrix44 transpose() const;

	T_MATH_INLINE Matrix44 inverse() const;

	T_MATH_INLINE Matrix44 inverseOrtho() const;

	T_MATH_INLINE Matrix44& operator = (const Matrix44& m);

	T_MATH_INLINE Matrix44& operator += (const Matrix44& m);

	T_MATH_INLINE Matrix44& operator -= (const Matrix44& m);

	T_MATH_INLINE Matrix44& operator *= (const Matrix44& m);

	T_MATH_INLINE bool operator == (const Matrix44& m) const;

	T_MATH_INLINE bool operator != (const Matrix44& m) const;

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Matrix44 operator + (const Matrix44& lh, const Matrix44& rh);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Matrix44 operator - (const Matrix44& lh, const Matrix44& rh);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector4 operator * (const Matrix44& m, const Vector4& v);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Matrix44 operator * (const Matrix44& lh, const Matrix44& rh);
};

T_MATH_INLINE T_DLLCLASS Matrix44 orthoLh(float width, float height, float zn, float zf);

T_MATH_INLINE T_DLLCLASS Matrix44 orthoLh(float left, float top, float right, float bottom, float zn, float zf);

T_MATH_INLINE T_DLLCLASS Matrix44 orthoRh(float width, float height, float zn, float zf);

T_MATH_INLINE T_DLLCLASS Matrix44 perspectiveLh(float fov, float aspect, float zn, float zf);

T_MATH_INLINE T_DLLCLASS Matrix44 perspectiveRh(float fov, float aspect, float zn, float zf);

T_MATH_INLINE T_DLLCLASS Matrix44 translate(const Vector4& t);

T_MATH_INLINE T_DLLCLASS Matrix44 translate(float x, float y, float z);

T_MATH_INLINE T_DLLCLASS Matrix44 rotateX(float angle);

T_MATH_INLINE T_DLLCLASS Matrix44 rotateY(float angle);

T_MATH_INLINE T_DLLCLASS Matrix44 rotateZ(float angle);

T_MATH_INLINE T_DLLCLASS Matrix44 rotate(const Vector4& axis, float angle);

T_MATH_INLINE T_DLLCLASS Matrix44 scale(const Vector4& s);

T_MATH_INLINE T_DLLCLASS Matrix44 scale(float x, float y, float z);

T_MATH_INLINE T_DLLCLASS Matrix44 lookAt(const Vector4& position, const Vector4& target, const Vector4& up = Vector4(0.0f, 1.0f, 0.0f, 0.0f));

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Matrix44.inl"
#endif

#endif	// traktor_Matrix44_H
