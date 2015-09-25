#include "Core/Misc/SafeDestroy.h"
#include "World/IEntityComponent.h"
#include "World/Entity/ComponentEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntity", ComponentEntity, Entity)

ComponentEntity::ComponentEntity()
:	m_visible(true)
{
}

void ComponentEntity::destroy()
{
	safeDestroy(m_entity);
}

void ComponentEntity::setTransform(const Transform& transform)
{
	if (m_entity)
		m_entity->setTransform(transform);
}

bool ComponentEntity::getTransform(Transform& outTransform) const
{
	if (m_entity)
		return m_entity->getTransform(outTransform);
	else
		return false;
}

Aabb3 ComponentEntity::getBoundingBox() const
{
	if (m_entity)
		return m_entity->getBoundingBox();
	else
		return Aabb3();
}

void ComponentEntity::update(const UpdateParams& update)
{
	for (RefArray< IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->update(update);

	if (m_entity)
		m_entity->update(update);
}

	}
}
