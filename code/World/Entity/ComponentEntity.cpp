#include "World/IEntityComponent.h"
#include "World/WorldContext.h"
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
	for (SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->destroy();
	m_components.clear();
}

void ComponentEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
	for (SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->setTransform(transform);
}

bool ComponentEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 ComponentEntity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		boundingBox.contain(i->second->getBoundingBox());
	return boundingBox;
}

void ComponentEntity::update(const UpdateParams& update)
{
	for (SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->update(update);
}

void ComponentEntity::setComponent(IEntityComponent* component)
{
	m_components[&type_of(component)] = component;
}

IEntityComponent* ComponentEntity::getComponent(const TypeInfo& componentType) const
{
	SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.find(&componentType);
	return i != m_components.end() ? i->second : 0;
}

void ComponentEntity::render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass)
{
	for (SmallMap< const TypeInfo*, Ref< IEntityComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		worldContext.build(worldRenderView, worldRenderPass, i->second);
}

	}
}
