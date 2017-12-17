/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CharacterComponentData", 0, CharacterComponentData, world::IEntityComponentData)

CharacterComponentData::CharacterComponentData()
{
}

CharacterComponentData::CharacterComponentData(BodyDesc* bodyDesc)
:	m_bodyDesc(bodyDesc)
{
}

CharacterComponentData::CharacterComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide)
:	m_bodyDesc(bodyDesc)
,	m_eventCollide(eventCollide)
{
}

Ref< CharacterComponent > CharacterComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setEnable(false);

	Ref< world::IEntityEvent > eventCollide;
	if (m_eventCollide)
	{
		eventCollide = entityBuilder->create(m_eventCollide);
		if (!eventCollide)
			return 0;
	}

	return new CharacterComponent(body, eventManager, eventCollide);
}

void CharacterComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::IEntityEventData >(L"eventCollide", m_eventCollide);
}

	}
}
