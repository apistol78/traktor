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
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "World/IEntityComponent.h"

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

/*! Plant feet on the ground beneath the character.
 * \ingroup Animation
 *
 * Each foot is traced against the physics world and moved onto the surface
 * with a two bone (hip, knee, foot) analytic IK solve so the leg keeps its
 * length. The pelvis is lowered first by the deepest foot so the lower leg
 * can reach without stretching. A foot the animation has lifted is left
 * alone; the correction is weighted by the animated sole height so swing
 * phases aren't dragged onto the ground. Offsets and surface normals are
 * smoothed over time and the correction fades out with distance.
 */
class T_DLLCLASS FootPlacementComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	struct Settings
	{
		render::handle_t hipsJoint = 0;
		uint32_t traceInclude = 0;
		uint32_t traceIgnore = 0;
		float offset = 0.1f;
		float rangeUp = 0.3f;
		float rangeDown = 0.6f;
		float pelvisDropMax = 0.35f;
		float liftHeight = 0.08f;
		float smoothTime = 0.08f;
		float maxTilt = 0.5f;
		float cullDistance = 10.0f;
	};

	explicit FootPlacementComponent(
		physics::PhysicsManager* physicsManager,
		const AlignedVector< render::handle_t >& footJoints,
		const Settings& settings
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	/*! Smoothed per foot state, carried between updates. */
	struct FootState
	{
		float offset = 0.0f;		//!< Vertical correction of the foot, object space along up.
		Vector4 normal;				//!< Ground normal in object space, up when not planted.
	};

	world::Entity* m_owner = nullptr;
	Ref< physics::PhysicsManager > m_physicsManager;
	AlignedVector< render::handle_t > m_footJoints;
	AlignedVector< FootState > m_footStates;
	Settings m_settings;
	float m_pelvisOffset = 0.0f;
};

}
