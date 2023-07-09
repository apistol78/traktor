/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallSet.h"
#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class CollisionSpecification;

}

namespace traktor::animation
{

/*! Ragdoll pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	RagDollPoseControllerData();

	explicit RagDollPoseControllerData(
		const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionGroup,
		const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionMask
	);

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const { return m_collisionGroup; }

	const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const { return m_collisionMask; }

	const IPoseControllerData* getTrackPoseController() const { return m_trackPoseController; }

private:
	friend class RagDollPoseController;

	SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionMask;
	bool m_autoDeactivate = false;
	bool m_enabled = true;
	float m_limbMass = 1.0f;
	float m_linearDamping = 0.1f;
	float m_angularDamping = 0.1f;
	float m_linearThreshold = 0.8f;
	float m_angularThreshold = 1.0f;
	uint32_t m_constraintAxises = 0;
	Ref< const IPoseControllerData > m_trackPoseController;
	float m_trackLinearTension = 0.0f;
	float m_trackAngularTension = 0.0f;
	float m_trackDuration = 0.0f;
};

}
