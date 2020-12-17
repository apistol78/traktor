#include "Scene/Editor/ExternalEntityBuilder.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "World/EntityData.h"
#include "World/IEntityFactory.h"
#include "World/Editor/EditorAttributesComponentData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ExternalEntityBuilder", ExternalEntityBuilder, world::IEntityBuilder)

ExternalEntityBuilder::ExternalEntityBuilder(world::IEntityBuilder* entityBuilder)
:	m_entityBuilder(entityBuilder)
{
}

void ExternalEntityBuilder::addFactory(const world::IEntityFactory* entityFactory)
{
	m_entityBuilder->addFactory(entityFactory);
}

void ExternalEntityBuilder::removeFactory(const world::IEntityFactory* entityFactory)
{
	m_entityBuilder->removeFactory(entityFactory);
}

const world::IEntityFactory* ExternalEntityBuilder::getFactory(const world::EntityData* entityData) const
{
	return m_entityBuilder->getFactory(entityData);
}

const world::IEntityFactory* ExternalEntityBuilder::getFactory(const world::IEntityEventData* entityEventData) const
{
	return m_entityBuilder->getFactory(entityEventData);
}

const world::IEntityFactory* ExternalEntityBuilder::getFactory(const world::IEntityComponentData* entityComponentData) const
{
	return m_entityBuilder->getFactory(entityComponentData);
}

Ref< world::Entity > ExternalEntityBuilder::create(const world::EntityData* entityData) const
{
	// Get visibility state from editor attributes.
	if (auto editorAttributes = entityData->getComponent< world::EditorAttributesComponentData >())
	{
		if (!editorAttributes->visible)
			return nullptr;
	}

	const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityData);
	if (entityFactory)
		return entityFactory->createEntity(this, *entityData);
	else
		return nullptr;
}

Ref< world::IEntityEvent > ExternalEntityBuilder::create(const world::IEntityEventData* entityEventData) const
{
	const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityEventData);
	if (entityFactory)
		return entityFactory->createEntityEvent(this, *entityEventData);
	else
		return nullptr;
}

Ref< world::IEntityComponent > ExternalEntityBuilder::create(const world::IEntityComponentData* entityComponentData) const
{
	const world::IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityComponentData);
	if (entityFactory)
		return entityFactory->createEntityComponent(this, *entityComponentData);
	else
		return nullptr;
}

const world::IEntityBuilder* ExternalEntityBuilder::getCompositeEntityBuilder() const
{
	return this;
}

	}
}
