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
#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

}

namespace traktor::physics
{

class Body;
struct CollisionInfo;
class PhysicsManager;
class VehicleComponentData;
class Wheel;

/*! Vehicle simulation component.
 * \ingroup Physics
 */
class T_DLLCLASS VehicleComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit VehicleComponent(
		PhysicsManager* physicsManager,
		const VehicleComponentData* data,
		const RefArray< Wheel >& wheels,
		uint32_t traceInclude,
		uint32_t traceIgnore
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setMaxVelocity(float maxVelocity);

	float getMaxVelocity() const;

	void setSteerAngle(float steerAngle);

	float getSteerAngle() const;

	float getSteerAngleFiltered() const;

	void setEngineThrottle(float engineThrottle);

	float getEngineThrottle() const;

	void setEngineBoost(float engineBoost);

	float getEngineBoost() const;

	void setBreaking(float breaking);

	float getBreaking() const;

	const RefArray< Wheel >& getWheels() const { return m_wheels; }

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const VehicleComponentData > m_data;
	RefArray< Wheel > m_wheels;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_maxVelocity;
	float m_steerAngle;
	float m_steerAngleTarget;
	float m_engineThrottle;
	float m_engineBoost;
	float m_breaking;
	bool m_airBorn;

	void updateSteering(Body* body, float dT);

	void updateSuspension(Body* body, float dT);
	
	void updateGrip(Body* body, float dT);

	void updateFriction(Body* body, float dT);

	void updateEngine(Body* body, float dT);

	void updateWheels(Body* body, float dT);
};

}
