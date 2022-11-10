/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Sphere
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Sphere
{
public:
	Vector4 center;
	Scalar radius;

	Sphere() = default;

	Sphere(const Vector4& center_, const Scalar& radius_);

	bool inside(const Vector4& pt) const;

	bool intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistance) const;

	bool intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistanceEnter, Scalar& outDistanceExit) const;
};

}
