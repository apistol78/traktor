#include "World/WorldContext.h"
#include "World/Entity/EntityUpdate.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/TransientEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.TransientEntity", TransientEntity, SpatialEntity)

TransientEntity::TransientEntity(
	GroupEntity* parentGroup,
	SpatialEntity* visualEntity,
	float duration
)
:	m_parentGroup(parentGroup)
,	m_otherEntity(visualEntity)
,	m_duration(duration)
{
}

void TransientEntity::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
	worldContext.build(worldRenderView, worldRenderPass, m_otherEntity);
}

void TransientEntity::update(const EntityUpdate* update)
{
	if ((m_duration -= update->getDeltaTime()) <= 0.0f)
		m_parentGroup->removeEntity(this);
	
	m_otherEntity->update(update);
}

void TransientEntity::setTransform(const Transform& transform)
{
	m_otherEntity->setTransform(transform);
}

bool TransientEntity::getTransform(Transform& outTransform) const
{
	return m_otherEntity->getTransform(outTransform);
}

Aabb3 TransientEntity::getBoundingBox() const
{
	return m_otherEntity->getBoundingBox();
}

	}
}
