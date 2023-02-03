/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/Body.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CharacterComponentData", 3, CharacterComponentData, world::IEntityComponentData)

Ref< CharacterComponent > CharacterComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	uint32_t traceInclude = 0;
	uint32_t traceIgnore = 0;

	for (const auto& traceIncludeIt : m_traceInclude)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(traceIncludeIt, traceGroup))
			return nullptr;
		traceInclude |= traceGroup->getBitMask();
	}
	for (const auto& traceIgnoreId : m_traceIgnore)
	{
		resource::Proxy< CollisionSpecification > traceGroup;
		if (!resourceManager->bind(traceIgnoreId, traceGroup))
			return nullptr;
		traceIgnore |= traceGroup->getBitMask();
	}

	StaticBodyDesc bodyDesc;

	bodyDesc.setShape(getShapeDesc(0.0f));
	bodyDesc.setFriction(0.0f);
	bodyDesc.setKinematic(true);

	Ref< Body > bodyWide = physicsManager->createBody(resourceManager, &bodyDesc);
	if (!bodyWide)
		return nullptr;

	bodyDesc.setShape(getShapeDesc(0.2f * m_radius));
	bodyDesc.setFriction(0.0f);
	bodyDesc.setKinematic(true);

	Ref< Body > bodySlim = physicsManager->createBody(resourceManager, &bodyDesc);
	if (!bodySlim)
		return nullptr;

	bodyWide->setEnable(false);
	bodySlim->setEnable(false);

	return new CharacterComponent(
		physicsManager,
		this,
		bodyWide,
		bodySlim,
		traceInclude,
		traceIgnore
	);
}

Ref< ShapeDesc > CharacterComponentData::getShapeDesc(float epsilon) const
{
	Ref< CapsuleShapeDesc > shapeDesc = new CapsuleShapeDesc();
	shapeDesc->setCollisionGroup(m_collisionGroup);
	shapeDesc->setLocalTransform(Transform(
		Quaternion::fromEulerAngles(0.0f, deg2rad(90.0f), 0.0f)
	));
	shapeDesc->setRadius(m_radius - epsilon);
	shapeDesc->setLength(m_height);
	return shapeDesc;
}

int32_t CharacterComponentData::getOrdinal() const
{
	return -100;
}

void CharacterComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void CharacterComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< CharacterComponentData >() >= 3)
		s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"collisionGroup", m_collisionGroup);

	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"height", m_height, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"step", m_step, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	if (s.getVersion< CharacterComponentData >() >= 1)
		s >> Member< float >(L"jumpImpulse", m_jumpImpulse, AttributeRange(0.0f) | AttributeUnit(UnitType::NewtonSecond));

	if (s.getVersion< CharacterComponentData >() >= 2)
	{
		s >> Member< float >(L"maxVelocity", m_maxVelocity, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres, true));
		s >> Member< float >(L"velocityDamping", m_velocityDamping, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
	}
}

}
