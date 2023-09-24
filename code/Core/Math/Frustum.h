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
#include "Core/Containers/StaticVector.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix44;

/*! Frustum
 * \ingroup Core
 */
class T_DLLCLASS Frustum
{
public:
	enum PlaneSide
	{
		PsLeft,
		PsRight,
		PsBottom,
		PsTop,
		PsNear,
		PsFar
	};

	enum InsideResult
	{
		IrOutside = 0,
		IrInside = 1,
		IrPartial = 2
	};

	StaticVector< Plane, 12 > planes;
	Vector4 corners[8];
	Vector4 center;

	void buildFromPlanes(const Plane planes_[6]);

	void buildFromCorners(const Vector4 corners_[8]);

	void buildPerspective(float vfov, float aspect, float zn, float zf);

	void buildOrtho(float width, float height, float zn, float zf);

	void setNearZ(const Scalar& zn);

	Scalar getNearZ() const;

	void setFarZ(const Scalar& zf);

	Scalar getFarZ() const;

	void scale(const Scalar& f);

	InsideResult inside(const Vector4& point) const;

	InsideResult inside(const Vector4& center_, const Scalar& radius) const;

	InsideResult inside(const Aabb3& aabb) const;

	InsideResult inside(const Matrix44& transform, const Frustum& other) const;

private:
	void update();
};

}

