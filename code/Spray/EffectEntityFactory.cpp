#include "Resource/IResourceManager.h"
#include "Sound/Sound.h"
#include "Spray/Effect.h"
#include "Spray/EffectEntity.h"
#include "Spray/EffectEntityData.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/SoundEvent.h"
#include "Spray/SoundEventData.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntityFactory", EffectEntityFactory, world::IEntityFactory)

EffectEntityFactory::EffectEntityFactory(resource::IResourceManager* resourceManager, world::IEntityEventManager* eventManager, sound::ISoundPlayer* soundPlayer)
:	m_resourceManager(resourceManager)
,	m_eventManager(eventManager)
,	m_soundPlayer(soundPlayer)
{
}

const TypeInfoSet EffectEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectEntityData >());
	return typeSet;
}

const TypeInfoSet EffectEntityFactory::getEntityEventTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SoundEventData >());
	typeSet.insert(&type_of< SpawnEffectEventData >());
	return typeSet;
}

Ref< world::Entity > EffectEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const EffectEntityData* >(&entityData)->createEntity(m_resourceManager, m_eventManager, m_soundPlayer);
}

Ref< world::IEntityEvent > EffectEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	if (const SoundEventData* soundEventData = dynamic_type_cast< const SoundEventData* >(&entityEventData))
	{
		resource::Proxy< sound::Sound > sound;
		if (!m_resourceManager->bind(soundEventData->m_sound, sound))
			return 0;

		return new SoundEvent(m_soundPlayer, sound, soundEventData->m_positional, soundEventData->m_follow);
	}
	else if (const SpawnEffectEventData* spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(&entityEventData))
	{
		resource::Proxy< Effect > effect;
		if (!m_resourceManager->bind(spawnEventData->getEffect(), effect))
			return 0;

		return new SpawnEffectEvent(
			m_soundPlayer,
			effect,
			spawnEventData->getFollow(),
			spawnEventData->getUseRotation()
		);
	}
	else
		return 0;
}

	}
}
