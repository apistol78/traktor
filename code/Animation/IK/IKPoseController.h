/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::animation
{

/*! IK pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit IKPoseController(
		physics::PhysicsManager* physicsManager,
		IPoseController* poseController,
		uint32_t solverIterations
	);

	virtual ~IKPoseController();

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

	IPoseController* getNeutralPoseController() const { return m_poseController; }

private:
	Ref< physics::PhysicsManager > m_physicsManager;
	Ref< IPoseController > m_poseController;
	uint32_t m_solverIterations;
};

}
