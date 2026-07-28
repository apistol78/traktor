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

/*!
 * \ingroup Animation
 */
class T_DLLCLASS FootPlacementComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit FootPlacementComponent(
		physics::PhysicsManager* physicsManager,
		const AlignedVector< render::handle_t >& footJoints,
		uint32_t traceInclude,
		uint32_t traceIgnore,
		float offset,
		float range
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	world::Entity* m_owner = nullptr;
	Ref< physics::PhysicsManager > m_physicsManager;
	AlignedVector< render::handle_t > m_footJoints;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_offset;
	float m_range;
};

}
