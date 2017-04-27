/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/Effect.h"
#include "Spray/EffectEntity.h"
#include "Spray/EffectEntityData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectEntityData", 0, EffectEntityData, world::EntityData)

Ref< EffectEntity > EffectEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityEventManager* eventManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< Effect > effect;
	if (!resourceManager->bind(m_effect, effect))
		return 0;

	return new EffectEntity(
		getTransform(),
		effect,
		eventManager,
		soundPlayer
	);
}

void EffectEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	s >> resource::Member< Effect >(L"effect", m_effect);
}

	}
}
