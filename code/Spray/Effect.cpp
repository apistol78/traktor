/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Effect", Effect, Object)

Effect::Effect(
	render::IRenderSystem* renderSystem,
	resource::IResourceManager* resourceManager,
	float duration,
	float loopStart,
	float loopEnd,
	const RefArray< EffectLayer >& layers
)
:	m_renderSystem(renderSystem)
,	m_resourceManager(resourceManager)
,	m_duration(duration)
,	m_loopStart(loopStart)
,	m_loopEnd(loopEnd)
,	m_layers(layers)
{
}

Ref< EffectInstance > Effect::createInstance() const
{
	Ref< EffectInstance > effectInstance = new EffectInstance(this);
	for (auto layer : m_layers)
	{
		Ref< EffectLayerInstance > layerInstance = layer->createInstance(m_renderSystem, m_resourceManager);
		if (layerInstance)
			effectInstance->m_layerInstances.push_back(layerInstance);
	}
	return effectInstance;
}

}
