#include "Core/Serialization/AttributeRange.h"
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

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CharacterComponentData", 0, CharacterComponentData, world::IEntityComponentData)

CharacterComponentData::CharacterComponentData()
:	m_radius(1.0f)
,	m_height(2.0f)
,	m_step(0.5f)
{
}

Ref< CharacterComponent > CharacterComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	StaticBodyDesc bodyDesc;

	bodyDesc.setShape(getShapeDesc(0.0f));
	bodyDesc.setFriction(0.0f);
	bodyDesc.setKinematic(true);

	Ref< Body > bodyWide = physicsManager->createBody(resourceManager, &bodyDesc);
	if (!bodyWide)
		return nullptr;

	bodyDesc.setShape(getShapeDesc(0.1f));
	bodyDesc.setFriction(0.0f);
	bodyDesc.setKinematic(true);

	Ref< Body > bodySlim = physicsManager->createBody(resourceManager, &bodyDesc);
	if (!bodySlim)
		return nullptr;

	bodyWide->setEnable(false);
	bodySlim->setEnable(false);

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
	shapeDesc->setLocalTransform(Transform(
		Quaternion::fromEulerAngles(0.0f, deg2rad(90.0f), 0.0f)
	));
	shapeDesc->setRadius(m_radius - epsilon);
	shapeDesc->setLength(m_height);
	return shapeDesc;
}

void CharacterComponentData::serialize(ISerializer& s)
{
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
	s >> Member< float >(L"height", m_height, AttributeRange(0.0f));
	s >> Member< float >(L"step", m_step, AttributeRange(0.0f));
}

	}
}
