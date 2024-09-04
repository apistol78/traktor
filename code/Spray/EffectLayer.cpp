/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/Sequence.h"
#include "Spray/Trail.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectLayer", EffectLayer, Object)

EffectLayer::EffectLayer(
	float time,
	float duration,
	Emitter* emitter,
	Trail* trail,
	Sequence* sequence,
	const world::IEntityEvent* triggerEnable,
	const world::IEntityEvent* triggerDisable
)
:	m_time(time)
,	m_duration(duration)
,	m_emitter(emitter)
,	m_trail(trail)
,	m_sequence(sequence)
,	m_triggerEnable(triggerEnable)
,	m_triggerDisable(triggerDisable)
{
}

Ref< EffectLayerInstance > EffectLayer::createInstance(resource::IResourceManager* resourceManager, GPUBufferPool* gpuBufferPool) const
{
	Ref< IEmitterInstance > emitterInstance;
	Ref< TrailInstance > trailInstance;
	Ref< SequenceInstance > sequenceInstance;

	if (m_emitter)
	{
		emitterInstance = m_emitter->createInstance(resourceManager, gpuBufferPool, m_duration);
		if (!emitterInstance)
			return nullptr;
	}

	if (m_trail)
	{
		trailInstance = m_trail->createInstance();
		if (!trailInstance)
			return nullptr;
	}

	if (m_sequence)
	{
		sequenceInstance = m_sequence->createInstance();
		if (!sequenceInstance)
			return nullptr;
	}

	return new EffectLayerInstance(
		this,
		emitterInstance,
		trailInstance,
		sequenceInstance
	);
}

}
