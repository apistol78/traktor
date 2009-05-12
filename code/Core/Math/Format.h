#ifndef traktor_Format_H
#define traktor_Format_H

#include "Core/Io/OutputStream.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"

namespace traktor
{

	/*! \brief Format helper of Vector2 instances.
	 * \ingroup Core
	 */
	OutputStream& operator << (OutputStream& os, const Vector2& v)
	{
		os << L"{ " << v.x << L", " << v.y << L" }";
		return os;
	}

	/*! \brief Format helper of Vector4 instances.
	 * \ingroup Core
	 */
	OutputStream& operator << (OutputStream& os, const Vector4& v)
	{
		os << L"{ " << v.x() << L", " << v.y() << L", " << v.z() << L", " << v.w() << L" }";
		return os;
	}

	/*! \brief Format helper of Quaternion instances.
	 * \ingroup Core
	 */
	OutputStream& operator << (OutputStream& os, const Quaternion& q)
	{
		os << L"{ " << q.x << L", " << q.y << L", " << q.z << L", " << q.w << L" }";
		return os;
	}

}

#endif	// traktor_Format_H
