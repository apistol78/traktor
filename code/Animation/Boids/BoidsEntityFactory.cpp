#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsComponentData.h"
#include "Animation/Boids/BoidsEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityFactory", BoidsEntityFactory, world::IEntityFactory)


const TypeInfoSet BoidsEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< BoidsComponentData >();
}

const TypeInfoSet BoidsEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet BoidsEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > BoidsEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > BoidsEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > BoidsEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return checked_type_cast< const BoidsComponentData* >(&entityComponentData)->createComponent(builder);
}

	}
}
