/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.VehicleComponentData", 0, VehicleComponentData, world::IEntityComponentData)

VehicleComponentData::VehicleComponentData()
:	m_steerAngleVelocity(deg2rad(150.0f))
,	m_fudgeDistance(0.2f)
,	m_swayBarForceCoeff(0.15f)
,	m_maxVelocity(30.0f)
,	m_engineForceCoeff(140.0f)
{
}

VehicleComponentData::VehicleComponentData(const BodyDesc* bodyDesc)
:	m_bodyDesc(bodyDesc)
,	m_steerAngleVelocity(deg2rad(150.0f))
,	m_fudgeDistance(0.2f)
,	m_swayBarForceCoeff(0.15f)
,	m_maxVelocity(30.0f)
,	m_engineForceCoeff(140.0f)
{
}

Ref< VehicleComponent > VehicleComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setEnable(false);

	RefArray< Wheel > wheels(m_wheels.size());
	for (uint32_t i = 0; i < m_wheels.size(); ++i)
		wheels[i] = new Wheel(m_wheels[i]);

	uint32_t traceInclude = 0;
	uint32_t traceIgnore = 0;

	for (std::set< resource::Id< CollisionSpecification > >::const_iterator i = m_traceInclude.begin(); i != m_traceInclude.end(); ++i)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(*i, traceGroup))
			return 0;
		traceInclude |= traceGroup->getBitMask();
	}
	for (std::set< resource::Id< CollisionSpecification > >::const_iterator i = m_traceIgnore.begin(); i != m_traceIgnore.end(); ++i)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(*i, traceGroup))
			return 0;
		traceIgnore |= traceGroup->getBitMask();
	}

	return new VehicleComponent(
		physicsManager,
		this,
		body,
		wheels,
		traceInclude,
		traceIgnore
	);
}

void VehicleComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< const BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRefArray< const WheelData >(L"wheels", m_wheels);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> Member< float >(L"steerAngleVelocity", m_steerAngleVelocity);
	s >> Member< float >(L"fudgeDistance", m_fudgeDistance);
	s >> Member< float >(L"swayBarForceCoeff", m_swayBarForceCoeff);
	s >> Member< float >(L"maxVelocity", m_maxVelocity);
	s >> Member< float >(L"engineForceCoeff", m_engineForceCoeff);
}

	}
}
