#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsComponentData.h"
#include "Animation/Boids/BoidsFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsFactory", BoidsFactory, world::IEntityFactory)


const TypeInfoSet BoidsFactory::getEntityTypes() const
{
	return makeTypeInfoSet< BoidsComponentData >();
}

const TypeInfoSet BoidsFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet BoidsFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > BoidsFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > BoidsFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > BoidsFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return checked_type_cast< const BoidsComponentData* >(&entityComponentData)->createComponent(builder);
}

	}
}
