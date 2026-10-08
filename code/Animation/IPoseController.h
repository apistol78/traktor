/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

}

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

	virtual void setOwner(world::Entity* owner) = 0;

	virtual void setTransform(const Transform& transform) = 0;

	/*! Get driven world transform of owner entity.
	 *
	 * \param outEntityTransform World transform of owner entity.
	 * \return True if this controller drives the owner entity's transform.
	 */
	virtual bool getEntityTransform(Transform& outEntityTransform) const { return false; }

	/*! Reset controller to match a given pose.
	 *
	 * \param worldTransform World transform of owner entity.
	 * \param skeleton Skeleton being posed.
	 * \param poseTransforms Object space joint transforms of the pose to match.
	 */
	virtual void reset(
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& poseTransforms) {}

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

	/*! Consume accumulated root motion played since last call. */
	virtual Vector4 consumeRootMotion() { return Vector4::zero(); }

	/*! Return the pose controller actually driving the pose right now. */
	virtual IPoseController* getActivePoseController() = 0;

	/*! */
	virtual void getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) = 0;
};

}
