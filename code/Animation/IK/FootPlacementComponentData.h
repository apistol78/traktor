/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/SmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

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

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::animation
{

class FootPlacementComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS FootPlacementComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< FootPlacementComponent > createComponent(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager
	) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	SmallSet< resource::Id< physics::CollisionSpecification > > m_traceInclude;
	SmallSet< resource::Id< physics::CollisionSpecification > > m_traceIgnore;
	AlignedVector< std::wstring > m_footJoints;
	std::wstring m_hipsJoint;		//!< Joint lowered so the deepest foot can reach; leave empty to disable.
	float m_offset = 0.1f;			//!< Foot joint offset from actual bottom of foot.
	float m_rangeUp = 0.3f;			//!< Trace range above the sole.
	float m_rangeDown = 0.6f;		//!< Trace range below the sole.
	float m_pelvisDropMax = 0.35f;	//!< Furthest the pelvis is lowered.
	float m_liftHeight = 0.08f;		//!< Animated sole height at which a foot counts as fully lifted.
	float m_smoothTime = 0.08f;		//!< Settle time of the foot correction.
	float m_maxTilt = 30.0f;		//!< Furthest a foot tilts onto a slope, in degrees.
	float m_cullDistance = 10.0f;	//!< No foot placement beyond this distance from the camera.
};

}
