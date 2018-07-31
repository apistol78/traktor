/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_VehicleComponent_H
#define traktor_physics_VehicleComponent_H

#include "Core/RefArray.h"
#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;
struct CollisionInfo;
class PhysicsManager;
class VehicleComponentData;
class Wheel;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS VehicleComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	VehicleComponent(
		PhysicsManager* physicsManager,
		const VehicleComponentData* data,
		Body* body,
		const RefArray< Wheel >& wheels,
		uint32_t traceInclude,
		uint32_t traceIgnore
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void setSteerAngle(float steerAngle);

	float getSteerAngle() const;

	void setEngineThrottle(float engineThrottle);

	float getEngineThrottle() const;

	const RefArray< Wheel >& getWheels() const { return m_wheels; }

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const VehicleComponentData > m_data;
	Ref< Body > m_body;
	RefArray< Wheel > m_wheels;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_totalMass;
	float m_steerAngle;
	float m_steerAngleTarget;
	float m_engineThrottle;
	bool m_airBorn;

	void updateSteering(float dT);

	void updateSuspension(float dT);

	void updateFriction(float dT);

	void updateEngine(float dT);

	void updateWheels(float dT);
};

	}
}

#endif	// traktor_physics_VehicleComponent_H
