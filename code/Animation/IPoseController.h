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
	virtual void destroy() = 0;

	virtual void setTransform(const Transform& transform) = 0;

	/*! Evaluate pose through pose controller.
	 *
	 * \param time Current animation time.
	 * \param deltaTime Delta time since last evaluation.
	 * \param worldTransform World transform of owner entity.
	 * \param skeleton Skeleton of skinned mesh.
	 * \param jointTransforms Array of joint transforms in object space.
	 * \param outPoseTransforms Output pose transforms for each joint.
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
};

}
