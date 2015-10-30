#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Boids/BoidsEntityData.h"
#include "Animation/Boids/BoidsEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityFactory", BoidsEntityFactory, world::IEntityFactory)


const TypeInfoSet BoidsEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BoidsEntityData >());
	return typeSet;
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
	return checked_type_cast< const BoidsEntityData* >(&entityData)->createEntity(builder);
}

Ref< world::IEntityEvent > BoidsEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > BoidsEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	return 0;
}

	}
}
