#pragma once

#include "Core/Io/OutputStream.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

namespace traktor
{

	/*! Format helper of Matrix44 instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Matrix44& v)
	{
		os << L"(Matrix44)";
		return os;
	}

	/*! Format helper of Vector2 instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Vector2& v)
	{
		os << v.x << L", " << v.y;
		return os;
	}

	/*! Format helper of Vector4 instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Vector4& v)
	{
		os << v.x() << L", " << v.y() << L", " << v.z() << L", " << v.w();
		return os;
	}

	/*! Format helper of Quaternion instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Quaternion& q)
	{
		return os << q.e;
	}

}

