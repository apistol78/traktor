/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Matrix44.h"

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

//@}

}
