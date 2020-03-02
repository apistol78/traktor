#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/PathEntity/PathEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityFactory", PathEntityFactory, world::IEntityFactory)

const TypeInfoSet PathEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PathEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PathEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< PathComponentData >();
}

Ref< world::Entity > PathEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > PathEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > PathEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
		return pathComponentData->createComponent();
	else
		return nullptr;
}

	}
}
