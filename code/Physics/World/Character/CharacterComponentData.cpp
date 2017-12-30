/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CharacterComponentData", 0, CharacterComponentData, world::IEntityComponentData)

CharacterComponentData::CharacterComponentData()
:	m_stepHeight(1.0f)
{
}

CharacterComponentData::CharacterComponentData(BodyDesc* bodyDesc)
:	m_bodyDesc(bodyDesc)
,	m_stepHeight(1.0f)
{
}

Ref< CharacterComponent > CharacterComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setEnable(false);

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

	return new CharacterComponent(
		physicsManager,
		this,
		body,
		traceInclude,
		traceIgnore
	);
}

void CharacterComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> Member< float >(L"stepHeight", m_stepHeight, AttributeRange(0.0f));
}

	}
}
