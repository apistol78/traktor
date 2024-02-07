/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectComponentData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectComponentData", 0, EffectComponentData, world::IEntityComponentData)

EffectComponentData::EffectComponentData(const resource::Id< Effect >& effect)
:	m_effect(effect)
{
}

Ref< EffectComponent > EffectComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< Effect > effect;
	if (!resourceManager->bind(m_effect, effect))
		return nullptr;

	return new EffectComponent(effect);
}

int32_t EffectComponentData::getOrdinal() const
{
	return 0;
}

void EffectComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void EffectComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< Effect >(L"effect", m_effect);
}

}
