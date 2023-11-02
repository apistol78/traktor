/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.VehicleComponentData", 1, VehicleComponentData, world::IEntityComponentData)

VehicleComponentData::VehicleComponentData()
:	m_steerAngleVelocity(deg2rad(150.0f))
,	m_fudgeDistance(0.2f)
,	m_swayBarForce(0.15f)
,	m_maxVelocity(30.0f)
,	m_engineForce(140.0f)
,	m_breakingForce(20.0f)
{
}

Ref< VehicleComponent > VehicleComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	RefArray< Wheel > wheels(m_wheels.size());
	for (uint32_t i = 0; i < m_wheels.size(); ++i)
		wheels[i] = new Wheel(m_wheels[i]);

	uint32_t traceInclude = 0;
	uint32_t traceIgnore = 0;

	for (const auto& ti : m_traceInclude)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(ti, traceGroup))
			return nullptr;
		traceInclude |= traceGroup->getBitMask();
	}
	for (const auto& ti : m_traceIgnore)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(ti, traceGroup))
			return nullptr;
		traceIgnore |= traceGroup->getBitMask();
	}

	return new VehicleComponent(
		physicsManager,
		this,
		wheels,
		traceInclude,
		traceIgnore
	);
}

int32_t VehicleComponentData::getOrdinal() const
{
	return -100;
}

void VehicleComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void VehicleComponentData::serialize(ISerializer& s)
{
	s >> MemberRefArray< const WheelData >(L"wheels", m_wheels);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> Member< float >(L"steerAngleVelocity", m_steerAngleVelocity, AttributeUnit(UnitType::Radians, true));
	s >> Member< float >(L"fudgeDistance", m_fudgeDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"swayBarForce", m_swayBarForce);
	s >> Member< float >(L"maxVelocity", m_maxVelocity, AttributeUnit(UnitType::Metres, true));
	s >> Member< float >(L"engineForce", m_engineForce);
		if (s.getVersion< VehicleComponentData >() >= 1)
		s >> Member< float >(L"breakingForce", m_breakingForce);
}

}
