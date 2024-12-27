/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Float.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>

namespace traktor::physics
{

#if !defined(T_CONVERT_ASSERT)
#	define T_CONVERT_ASSERT T_ASSERT
#endif

/*! \ingroup Physics */
//@{

inline JPH::Vec3 convertToJolt(const Vector4& v)
{
	return JPH::Vec3(v.x(), v.y(), v.z());
}

inline JPH::Quat convertToJolt(const Quaternion& q)
{
    return JPH::Quat(q.e.x(), q.e.y(), q.e.z(), q.e.w());
}

inline Vector4 convertFromJolt(const JPH::Vec3& v, float w = 0.0f)
{
    return Vector4(v.GetX(), v.GetY(), v.GetZ(), w);
}

inline Quaternion convertFromJolt(const JPH::Quat& q)
{
    return Quaternion(q.GetX(), q.GetY(), q.GetZ(), q.GetW());
}

inline Transform convertFromJolt(const JPH::RMat44& transform)
{
    return Transform(
        convertFromJolt(transform.GetTranslation(), 1.0f),
        convertFromJolt(transform.GetQuaternion())
    );
}

//@}

}
