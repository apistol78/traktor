/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/TrailComponent.h"

#include "Spray/TrailInstance.h"
#include "World/Entity.h"

namespace traktor::spray
{
namespace
{

const float c_maxDeltaTime = 1.0f / 30.0f;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.TrailComponent", TrailComponent, world::IEntityComponent)

TrailComponent::TrailComponent(TrailInstance* trailInstance)
	: m_trailInstance(trailInstance)
{
}

void TrailComponent::destroy()
{
	m_trailInstance = nullptr;
	m_context.owner = nullptr;
}

void TrailComponent::setOwner(world::Entity* owner)
{
	m_context.owner = owner;
}

void TrailComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 TrailComponent::getBoundingBox() const
{
	if (!m_trailInstance)
		return Aabb3();

	const Aabb3& worldBoundingBox = m_trailInstance->getBoundingBox();
	if (worldBoundingBox.empty())
		return Aabb3();

	return worldBoundingBox.transform(m_transform.inverse());
}

void TrailComponent::update(const world::UpdateParams& update)
{
	if (!m_trailInstance)
		return;

	m_context.deltaTime = min((float)update.deltaTime, c_maxDeltaTime);
	m_trailInstance->update(m_context, m_transform, m_enable);
}

void TrailComponent::build(
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	TrailRenderer* trailRenderer) const
{
	if (m_trailInstance)
		m_trailInstance->build(worldRenderView, worldRenderPass, trailRenderer, m_transform);
}

void TrailComponent::addPoint(const Vector4& position)
{
	if (m_trailInstance)
		m_trailInstance->addPoint(m_context, position);
}

}
