/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;
class IEntityBuilder;

}

namespace traktor::physics
{

class CharacterComponent;
class CollisionSpecification;
class PhysicsManager;
class ShapeDesc;

/*! Character component data.
 * \ingroup Physics
 */
class T_DLLCLASS CharacterComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	/*! Create instance of character component. */
	Ref< CharacterComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	Ref< ShapeDesc > getShapeDesc(float epsilon) const;

	const std::set< resource::Id< CollisionSpecification > >& getTraceInclude() const { return m_traceInclude; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceIgnore() const { return m_traceIgnore; }

	/*! Get character radius. */
	float getRadius() const { return m_radius; }

	/*! Get character height. */
	float getHeight() const { return m_height; }

	/*! Get character step height. */
	float getStep() const { return m_step; }

	/*! Get character jump impulse. */
	float getJumpImpulse() const { return m_jumpImpulse; }

	/*! Get character max velocity. */
	float getMaxVelocity() const { return m_maxVelocity; }

	/*! Get velocity damping coefficient. */
	float getVelocityDamping() const { return m_velocityDamping; }

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::set< resource::Id< CollisionSpecification > > m_traceInclude;
	std::set< resource::Id< CollisionSpecification > > m_traceIgnore;
	float m_radius = 1.0f;
	float m_height = 2.0f;
	float m_step = 0.5f;
	float m_jumpImpulse = 1.0f;
	float m_maxVelocity = 2.0f;
	float m_velocityDamping = 1.0f;
};

}
