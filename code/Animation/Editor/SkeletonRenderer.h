/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::animation
{

class Skeleton;
class SkeletonComponent;

/*! \ingroup Animation */
//@{

/*! Draw skeleton bind and/or current pose for debugging.
 *
 * \param primitiveRenderer Renderer used to draw the debug primitives.
 * \param skeletonComponent Skeleton component providing the skeleton and its joint/pose transforms.
 * \param worldTransform World transform applied to the drawn primitives.
 * \param drawBind Draw the skeleton in bind pose.
 * \param drawPose Draw the skeleton in its current animated pose.
 */
void T_DLLCLASS drawSkeleton(
	render::PrimitiveRenderer* primitiveRenderer,
	const SkeletonComponent* skeletonComponent,
	const Matrix44& worldTransform,
	bool drawBind,
	bool drawPose
);

/*! Draw skeleton for debugging.
 *
 * \param primitiveRenderer Renderer used to draw the debug primitives.
 * \param skeleton Skeleton
 * \param worldTransform World transform applied to the drawn primitives.
 * \param color Color for drawing.
 */
void T_DLLCLASS drawSkeleton(
	render::PrimitiveRenderer* primitiveRenderer,
	const Skeleton* skeleton,
	const Matrix44& worldTransform,
	const Color4ub& color
);

/*! Draw skeleton for debugging.
 *
 * \param primitiveRenderer Renderer used to draw the debug primitives.
 * \param skeleton Skeleton
 * \param worldTransform World transform applied to the drawn primitives.
 * \param transforms Skeleton joint transforms.
 * \param color Color for drawing.
 */
void T_DLLCLASS drawSkeleton(
	render::PrimitiveRenderer* primitiveRenderer,
	const Skeleton* skeleton,
	const Matrix44& worldTransform,
	const AlignedVector< Transform >& transforms,
	const Color4ub& color
);

//@}

}
