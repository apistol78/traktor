#include "Spray/EffectEntity.h"
#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntityFactory.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntityFactory", EffectEntityFactory, world::IEntityFactory)

EffectEntityFactory::EffectEntityFactory(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer)
:	m_resourceManager(resourceManager)
,	m_soundPlayer(soundPlayer)
{
}

const TypeInfoSet EffectEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectEntityData >());
	return typeSet;
}

Ref< world::Entity > EffectEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const EffectEntityData* >(&entityData)->createEntity(m_resourceManager, m_soundPlayer);
}

	}
}
