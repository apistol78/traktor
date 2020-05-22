#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/PathEntity/PathFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathFactory", PathFactory, world::IEntityFactory)

const TypeInfoSet PathFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PathFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PathFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< PathComponentData >();
}

Ref< world::Entity > PathFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > PathFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > PathFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
		return pathComponentData->createComponent();
	else
		return nullptr;
}

	}
}
