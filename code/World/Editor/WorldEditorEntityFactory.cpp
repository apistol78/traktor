#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/WorldEditorEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEditorEntityFactory", WorldEditorEntityFactory, IEntityFactory)

const TypeInfoSet WorldEditorEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WorldEditorEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WorldEditorEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< EditorAttributesComponentData >();
}

Ref< Entity > WorldEditorEntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	return nullptr;
}

Ref< IEntityEvent > WorldEditorEntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< IEntityComponent > WorldEditorEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
