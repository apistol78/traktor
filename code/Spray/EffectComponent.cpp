/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Shader.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/Emitter.h"
#include "Spray/Trail.h"
#include "World/Entity.h"

namespace traktor::spray
{
	namespace
	{

const float c_maxDeltaTime = 1.0f / 30.0f;
const uint32_t c_updateDenom = 1;
Random g_randomSeed;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectComponent", EffectComponent, world::IEntityComponent)

EffectComponent::EffectComponent(const resource::Proxy< Effect >& effect)
:	m_transform(Transform::identity())
,	m_effect(effect)
,	m_counter(0)
,	m_enable(true)
{
	m_context.deltaTime = 0.0f;
	m_context.random = RandomGeometry(g_randomSeed.next());
}

EffectComponent::EffectComponent(const resource::Proxy< Effect >& effect, EffectInstance* effectInstance, const Context& context)
:	m_transform(Transform::identity())
,	m_effect(effect)
,	m_effectInstance(effectInstance)
,	m_context(context)
,	m_counter(0)
,	m_enable(true)
{
	// Do not recreate instance if we've been provided one.
	if (effectInstance != nullptr)
		m_effect.consume();

	updateTechniques();
}

void EffectComponent::destroy()
{
	if (m_effectInstance)
	{
		m_effectInstance->setLoopEnable(false);
		m_effectInstance->synchronize();
		m_effectInstance = nullptr;
	}

	m_effect.clear();
	m_techniques.clear();

	m_context.virtualSourceCallback = nullptr;
}

void EffectComponent::setOwner(world::Entity* owner)
{
	m_context.owner = owner;
}

void EffectComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 EffectComponent::getBoundingBox() const
{
	if (!m_effectInstance)
		return Aabb3();

	const Aabb3& worldBoundingBox = m_effectInstance->getBoundingBox();
	if (worldBoundingBox.empty())
		return Aabb3();

	return worldBoundingBox.transform(m_transform.inverse());
}

void EffectComponent::update(const world::UpdateParams& update)
{
	if ((m_counter++ % c_updateDenom) != 0)
		return;

	if (m_effect.changed() || !m_effectInstance)
	{
		m_effectInstance = m_effect->createInstance();
		if (m_effectInstance)
			updateTechniques();
		m_effect.consume();
	}

	if (m_effectInstance)
	{
		m_context.deltaTime = update.deltaTime;
		m_context.deltaTime = min(m_context.deltaTime, c_maxDeltaTime);
		m_context.deltaTime *= c_updateDenom;

		m_effectInstance->update(m_context, m_transform, m_enable);
	}
}

void EffectComponent::render(
	render::handle_t technique,
	const Vector4& cameraPosition,
	const Plane& cameraPlane,
	PointRenderer* pointRenderer,
	MeshRenderer* meshRenderer,
	TrailRenderer* trailRenderer
) const
{
	if (m_effectInstance)
		m_effectInstance->render(
			technique,
			pointRenderer,
			meshRenderer,
			trailRenderer,
			m_transform,
			cameraPosition,
			cameraPlane
		);
}

Aabb3 EffectComponent::getWorldBoundingBox() const
{
	return m_effectInstance ? m_effectInstance->getBoundingBox() : Aabb3();
}

void EffectComponent::reset()
{
	m_effectInstance = m_effect->createInstance();
}

void EffectComponent::setLoopEnable(bool loopEnable)
{
	if (m_effectInstance)
		m_effectInstance->setLoopEnable(loopEnable);
}

bool EffectComponent::getLoopEnable() const
{
	return m_effectInstance ? m_effectInstance->getLoopEnable() : false;
}

bool EffectComponent::isFinished() const
{
	if (!m_effect)
		return true;

	if (!m_effectInstance || m_effectInstance->getLoopEnable())
		return false;

	return m_effectInstance->getTime() >= m_effect->getDuration();
}

void EffectComponent::updateTechniques()
{
	// Collect set of techniques used by this effect; we store
	// these in a local set as we want to check against rendering
	// as fast as possible without going through every layer each time.
	m_techniques.clear();

	if (!m_effect)
		return;

	for (auto layer : m_effect->getLayers())
	{
		const Emitter* emitter = layer->getEmitter();
		if (emitter)
		{
			const resource::Proxy< render::Shader >& emitterShader = emitter->getShader();
			if (emitterShader)
				emitterShader->getTechniques(m_techniques);

			const resource::Proxy< mesh::InstanceMesh >& emitterMesh = emitter->getMesh();
			if (emitterMesh)
				emitterMesh->getTechniques(m_techniques);
		}

		const Trail* trail = layer->getTrail();
		if (trail)
		{
			const resource::Proxy< render::Shader >& trailShader = trail->getShader();
			if (trailShader)
				trailShader->getTechniques(m_techniques);
		}
	}
}

}
