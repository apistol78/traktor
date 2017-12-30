/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_VehicleComponentData_H
#define traktor_physics_VehicleComponentData_H

#include <set>
#include "Core/RefArray.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;
class IEntityBuilder;

	}

	namespace physics
	{

class BodyDesc;
class CollisionSpecification;
class PhysicsManager;
class VehicleComponent;
class WheelData;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS VehicleComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	VehicleComponentData();

	explicit VehicleComponentData(const BodyDesc* bodyDesc);

	Ref< VehicleComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	const RefArray< const WheelData >& getWheels() const { return m_wheels; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceInclude() const { return m_traceInclude; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceIgnore() const { return m_traceIgnore; }

	float getSteerAngleVelocity() const { return m_steerAngleVelocity; }

	float getFudgeDistance() const { return m_fudgeDistance; }

	float getSwayBarForceCoeff() const { return m_swayBarForceCoeff; }

	float getMaxVelocity() const { return m_maxVelocity; }

	float getEngineForceCoeff() const { return m_engineForceCoeff; }

private:
	Ref< const BodyDesc > m_bodyDesc;
	RefArray< const WheelData > m_wheels;
	std::set< resource::Id< CollisionSpecification > > m_traceInclude;
	std::set< resource::Id< CollisionSpecification > > m_traceIgnore;
	float m_steerAngleVelocity;
	float m_fudgeDistance;
	float m_swayBarForceCoeff;
	float m_maxVelocity;
	float m_engineForceCoeff;
};

	}
}

#endif	// traktor_physics_VehicleComponentData_H
