#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityFactory", PathEntityFactory, world::IEntityFactory)

const TypeInfoSet PathEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

const TypeInfoSet PathEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PathEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathComponentData >());
	return typeSet;
}

Ref< world::Entity > PathEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const PathEntityData* pathEntityData = checked_type_cast< const PathEntityData* >(&entityData);
	return pathEntityData->createEntity(builder);
}

Ref< world::IEntityEvent > PathEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > PathEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	if (const PathComponentData* pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
		return pathComponentData->createComponent(owner);

	return 0;
}

	}
}
