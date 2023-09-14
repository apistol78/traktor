/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Boids/BoidsComponent.h"
#include "Core/Math/RandomGeometry.h"
#include "World/Entity.h"
#include "World/WorldBuildContext.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::animation
{
	namespace
	{

RandomGeometry s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsComponent", BoidsComponent, world::IEntityComponent)

BoidsComponent::BoidsComponent(
	const Vector4& spawnVelocityDiagonal,
	const Vector4& constrain,
	float followForce,
	float repelDistance,
	float repelForce,
	float matchVelocityStrength,
	float centerForce,
	float maxVelocity
)
:	m_spawnVelocityDiagonal(spawnVelocityDiagonal)
,	m_constrain(constrain)
,	m_attractPosition(Vector4::zero())
,	m_followForce(followForce)
,	m_repelDistance(repelDistance)
,	m_repelForce(repelForce)
,	m_matchVelocityStrength(matchVelocityStrength)
,	m_centerForce(centerForce)
,	m_maxVelocity(maxVelocity)
{
}

void BoidsComponent::destroy()
{
	m_owner = nullptr;
}

void BoidsComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
	m_boids.resize(0);
}

void BoidsComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 BoidsComponent::getBoundingBox() const
{
	const Transform transformInv = m_transform.inverse();

	Aabb3 aabb;
	for (uint32_t i = 0; i < uint32_t(m_boids.size()); ++i)
		aabb.contain(transformInv * m_boids[i].position);

	return aabb;
}

void BoidsComponent::update(const world::UpdateParams& update)
{
	const Scalar deltaTime((float)min(update.deltaTime, 1.0 / 30.0));

	if (deltaTime <= FUZZY_EPSILON)
		return;

	world::GroupComponent* group = m_owner->getComponent< world::GroupComponent >();
	if (!group)
		return;

	const auto& entities = group->getEntities();

	// Ensure number of boids match number of entities.
	if (entities.size() != m_boids.size())
	{
		for (uint32_t i = (uint32_t)m_boids.size(); i < entities.size(); ++i)
		{
			auto& boid = m_boids.push_back();
			boid.position = entities[i]->getTransform().translation().xyz1();
			boid.velocity = s_random.nextUnit() * m_spawnVelocityDiagonal;
		}
		m_boids.resize(entities.size());
	}

	// Calculate perceived center and velocity of all boids.
	Vector4 center(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4 velocity(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		center += m_boids[i].position;
		velocity += m_boids[i].velocity;
	}

	const Scalar invBoidsSize(1.0f / (float(m_boids.size()) - 1.0f));

	// Update boids.
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		const Vector4 otherCenter = (center - m_boids[i].position) * invBoidsSize;
		const Vector4 otherVelocity = (velocity - m_boids[i].velocity) * invBoidsSize;

		// 1: Follow perceived center.
		m_boids[i].velocity += (otherCenter - m_boids[i].position) * m_followForce;

		// 2: Keep distance from other boids.
		for (uint32_t j = 0; j < uint32_t(m_boids.size()); ++j)
		{
			if (i != j)
			{
				Vector4 d = m_boids[j].position - m_boids[i].position;
				Scalar ln = d.normalize();
				if (ln > 0.0_simd && ln < m_repelDistance)
					m_boids[i].velocity -= d * m_repelForce;
			}
		}

		// 3: Try to match velocity with other boids.
		m_boids[i].velocity += (otherVelocity - m_boids[i].velocity) * m_matchVelocityStrength;

		// 4: Always try to be circulating around center.
		if (m_attractPosition.w() > 0.0_simd)
			m_boids[i].velocity += (m_attractPosition - m_boids[i].position).xyz0() * m_centerForce;

		// 5: Clamp velocity.
		const Scalar ln = m_boids[i].velocity.length();
		if (ln > 0.0_simd)
			m_boids[i].velocity = m_boids[i].velocity.normalized() * min(ln, m_maxVelocity);

		// Integrate position.
		m_boids[i].position += m_boids[i].velocity * deltaTime;

		// Constrain velocity.
		m_boids[i].velocity = m_boids[i].velocity * m_constrain;

		// Update boid entity.
		if (entities[i])
		{
			if (m_boids[i].velocity.length() > 0.0_simd)
				entities[i]->setTransform(Transform(
					lookAt(m_boids[i].position, m_boids[i].position + m_boids[i].velocity).inverse()
				));
			else
				entities[i]->setTransform(Transform(m_boids[i].position));
		}
	}
}

void BoidsComponent::setAttractPosition(const Vector4& attractPosition)
{
	m_attractPosition = attractPosition;
}

const Vector4& BoidsComponent::getAttractPosition() const
{
	return m_attractPosition;
}

}
