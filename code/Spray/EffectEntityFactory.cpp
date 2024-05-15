/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Resource/IResourceManager.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectComponentData.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/ListenerComponent.h"
#include "Spray/ListenerComponentData.h"
#include "Spray/SoundComponent.h"
#include "Spray/SoundComponentData.h"
#include "Spray/SoundEvent.h"
#include "Spray/SoundEventData.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventData.h"
#include "Spray/Feedback/EnvelopeFeedbackEvent.h"
#include "Spray/Feedback/EnvelopeFeedbackEventData.h"
#include "Spray/Feedback/OscillateFeedbackEvent.h"
#include "Spray/Feedback/OscillateFeedbackEventData.h"
#include "World/IEntityBuilder.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEntityFactory", 0, EffectEntityFactory, world::AbstractEntityFactory)

bool EffectEntityFactory::initialize(const ObjectStore& objectStore)
{
	m_resourceManager = objectStore.get< resource::IResourceManager >();
	m_soundPlayer = objectStore.get< sound::ISoundPlayer >();
	return true;
}

const TypeInfoSet EffectEntityFactory::getEntityEventTypes() const
{
	return makeTypeInfoSet<
		EnvelopeFeedbackEventData,
		OscillateFeedbackEventData,
		SoundEventData,
		SpawnEffectEventData
	>();
}

const TypeInfoSet EffectEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< 
		EffectComponentData,
		ListenerComponentData,
		SoundComponentData
	>();
}

Ref< world::IEntityEvent > EffectEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	if (auto envelopeFeedbackEventData = dynamic_type_cast< const EnvelopeFeedbackEventData* >(&entityEventData))
	{
		return new EnvelopeFeedbackEvent(envelopeFeedbackEventData);
	}
	else if (auto oscillateFeedbackEventData = dynamic_type_cast< const OscillateFeedbackEventData* >(&entityEventData))
	{
		return new OscillateFeedbackEvent(oscillateFeedbackEventData);
	}
	else if (auto soundEventData = dynamic_type_cast< const SoundEventData* >(&entityEventData))
	{
		resource::Proxy< sound::Sound > sound;
		if (!m_resourceManager->bind(soundEventData->m_sound, sound))
			return nullptr;

		return new SoundEvent(m_soundPlayer, sound, soundEventData->m_positional, soundEventData->m_follow, soundEventData->m_autoStopFar);
	}
	else if (auto spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(&entityEventData))
	{
		resource::Proxy< Effect > effect;
		if (!m_resourceManager->bind(spawnEventData->getEffect(), effect))
			return nullptr;

		return new SpawnEffectEvent(
			effect,
			spawnEventData->getTransform(),
			spawnEventData->getFollow(),
			spawnEventData->getUseRotation()
		);
	}
	else
		return nullptr;
}

Ref< world::IEntityComponent > EffectEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto effectComponentData = dynamic_type_cast< const EffectComponentData* >(&entityComponentData))
		return effectComponentData->createComponent(m_resourceManager);
	else if (auto listenerComponentData = dynamic_type_cast< const ListenerComponentData* >(&entityComponentData))
		return listenerComponentData->createComponent(m_soundPlayer);
	else if (auto soundComponentData = dynamic_type_cast< const SoundComponentData* >(&entityComponentData))
		return soundComponentData->createComponent(m_resourceManager, m_soundPlayer);
	else
		return nullptr;
}

}
