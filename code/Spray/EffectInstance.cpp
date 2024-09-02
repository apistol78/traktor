/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/EffectInstance.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Types.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectInstance", EffectInstance, Object)

EffectInstance::EffectInstance(const Effect* effect)
:	m_effect(effect)
,	m_time(0.0f)
{
	m_loopEnable = effect->getLoopEnd() > effect->getLoopStart();
}

void EffectInstance::update(Context& context, const Transform& transform, bool enable)
{
	m_time += context.deltaTime;
	if (m_loopEnable)
	{
		if (m_time >= m_effect->getLoopEnd())
			m_time = m_effect->getLoopStart();
	}

	m_boundingBox = Aabb3();
	for (auto layerInstance : m_layerInstances)
	{
		layerInstance->update(context, transform, m_time, enable);
		m_boundingBox.contain(layerInstance->getBoundingBox());
	}
}

void EffectInstance::synchronize()
{
	for (auto layerInstance : m_layerInstances)
		layerInstance->synchronize();
}

void EffectInstance::setup()
{
	for (auto layerInstance : m_layerInstances)
		layerInstance->setup();
}

void EffectInstance::render(
	render::handle_t technique,
	render::RenderContext* renderContext,
	PointRenderer* pointRenderer,
	MeshRenderer* meshRenderer,
	TrailRenderer* trailRenderer,
	const Transform& transform,
	const Vector4& cameraPosition,
	const Plane& cameraPlane
) const
{
	for (auto layerInstance : m_layerInstances)
	{
		layerInstance->render(
			technique,
			renderContext,
			pointRenderer,
			meshRenderer,
			trailRenderer,
			transform,
			cameraPosition,
			cameraPlane,
			m_time
		);
	}
}

}
