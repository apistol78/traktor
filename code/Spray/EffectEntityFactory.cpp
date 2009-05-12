#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntity.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntityFactory", EffectEntityFactory, world::EntityFactory)

const TypeSet EffectEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< EffectEntityData >());
	return typeSet;
}

world::Entity* EffectEntityFactory::createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const EffectEntityData* >(&entityData)->createEntity();
}

	}
}
