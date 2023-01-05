/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "World/IEntityComponent.h"

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
class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

}

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS BoidsComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit BoidsComponent(
		const RefArray< world::Entity >& boidEntities,
		const Vector4& spawnPositionDiagonal,
		const Vector4& spawnVelocityDiagonal,
		const Vector4& constrain,
		float followForce,
		float repelDistance,
		float repelForce,
		float matchVelocityStrength,
		float centerForce,
		float maxVelocity
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	const RefArray< world::Entity >& getEntities() const { return m_boidEntities; }

private:
	struct Boid
	{
		Vector4 position;
		Vector4 velocity;
	};

	RefArray< world::Entity > m_boidEntities;
	AlignedVector< Boid > m_boids;
	Transform m_transform;
	Vector4 m_constrain;
	Scalar m_followForce;
	Scalar m_repelDistance;
	Scalar m_repelForce;
	Scalar m_matchVelocityStrength;
	Scalar m_centerForce;
	Scalar m_maxVelocity;
};

}
