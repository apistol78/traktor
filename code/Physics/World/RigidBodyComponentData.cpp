/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.RigidBodyComponentData", 1, RigidBodyComponentData, world::IEntityComponentData)

RigidBodyComponentData::RigidBodyComponentData(BodyDesc* bodyDesc)
:	m_bodyDesc(bodyDesc)
{
}

RigidBodyComponentData::RigidBodyComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide)
:	m_bodyDesc(bodyDesc)
,	m_eventCollide(eventCollide)
{
}

Ref< RigidBodyComponent > RigidBodyComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return nullptr;

	body->setEnable(false);

	Ref< world::IEntityEvent > eventCollide;
	if (m_eventCollide)
	{
		eventCollide = entityBuilder->create(m_eventCollide);
		if (!eventCollide)
			return nullptr;
	}

	return new RigidBodyComponent(body, eventCollide, m_transformFilter);
}

int32_t RigidBodyComponentData::getOrdinal() const
{
	return -100;
}

void RigidBodyComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void RigidBodyComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::IEntityEventData >(L"eventCollide", m_eventCollide);

	if (s.getVersion< RigidBodyComponentData >() >= 1)
		s >> Member< float >(L"transformFilter", m_transformFilter, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

}
