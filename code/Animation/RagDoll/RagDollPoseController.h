/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Animation/IPoseController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;
class Joint;
class PhysicsManager;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace animation
	{

class RagDollPoseControllerData;

/*! Rag doll pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	RagDollPoseController();

	virtual ~RagDollPoseController();

	/*! Create rag-doll pose controller.
	 *
	 * \param initiallyDisabled If limbs should be initially disabled.
	 * \param fixateBones Fixate parent bones in world space.
	 */
	bool create(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const RagDollPoseControllerData* data,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		const AlignedVector< Transform >& jointTransforms,
		const AlignedVector< Velocity >& velocities,
		IPoseController* trackPoseController
	);

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& boneTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

	void setEnable(bool enable);

	bool isEnable() const;

	const RefArray< physics::Body >& getLimbs() const;

	const RefArray< physics::Joint >& getJoints() const;

private:
	Ref< IPoseController > m_trackPoseController;
	Scalar m_trackLinearTension;
	Scalar m_trackAngularTension;
	float m_trackDuration;
	RefArray< physics::Body > m_limbs;
	RefArray< physics::Joint > m_joints;
	Transform m_worldTransform;
	RefArray< physics::Body > m_deltaLimbs;
	AlignedVector< Transform > m_deltaTransforms;
	bool m_enable;
};

	}
}

