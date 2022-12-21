/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Pose;
class Skeleton;

/*! Pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IPoseController : public Object
{
	T_RTTI_CLASS;

public:
	struct Velocity
	{
		Vector4 linear;
		Vector4 angular;
	};

	virtual void destroy() = 0;

	virtual void setTransform(const Transform& transform) = 0;

	/*! Evaluate pose throught pose controller.
	 *
	 * \return True if pose is continuous since last evaluation.
	 */
	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) = 0;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) = 0;
};

}
