#include "World/IEntityComponent.h"
#include "World/WorldRenderView.h"
#include "World/Entity/ComponentEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntity", ComponentEntity, Entity)

ComponentEntity::ComponentEntity()
:	m_transform(Transform::identity())
{
}

void ComponentEntity::destroy()
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->destroy();
	m_components.clear();
}

void ComponentEntity::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

bool ComponentEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform.get();
	return true;
}

Aabb3 ComponentEntity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		boundingBox.contain((*i)->getBoundingBox());
	return boundingBox;
}

void ComponentEntity::update(const UpdateParams& update)
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->update(update);
}

void ComponentEntity::render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass)
{
	Transform transform = m_transform.get(worldRenderView.getInterval());
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->render(worldContext, worldRenderView, worldRenderPass, transform);
}

	}
}
