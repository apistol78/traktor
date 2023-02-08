/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "World/Entity.h"

namespace traktor::physics
{
	namespace
	{

const Vector4 c_101(1.0f, 0.0f, 1.0f);
const Vector4 c_010(0.0f, 1.0f, 0.0f);

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CharacterComponent", CharacterComponent, world::IEntityComponent)

CharacterComponent::CharacterComponent(
	PhysicsManager* physicsManager,
	const CharacterComponentData* data,
	Body* bodyWide,
	Body* bodySlim,
	uint32_t traceInclude,
	uint32_t traceIgnore
)
:	m_owner(nullptr)
,	m_physicsManager(physicsManager)
,	m_data(data)
,	m_bodyWide(bodyWide)
,	m_bodySlim(bodySlim)
,	m_traceInclude(traceInclude)
,	m_traceIgnore(traceIgnore)
,	m_headAngle(0.0f)
,	m_velocity(Vector4::zero())
,	m_impulse(Vector4::zero())
,	m_grounded(false)
{
}

void CharacterComponent::destroy()
{
	safeDestroy(m_bodyWide);
	safeDestroy(m_bodySlim);
	m_owner = nullptr;
}

void CharacterComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		const Transform transform = m_owner->getTransform();
		m_bodyWide->setTransform(transform * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
		m_bodyWide->setEnable(true);
	}
}

void CharacterComponent::setTransform(const Transform& transform)
{
	m_bodyWide->setTransform(transform * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
}

Aabb3 CharacterComponent::getBoundingBox() const
{
	return Aabb3();
}

void CharacterComponent::update(const world::UpdateParams& update)
{
	const Scalar dT(update.deltaTime);
	const Vector4 movement = m_velocity * dT;
	Vector4 position = m_bodyWide->getTransform().translation();

	// Add user impulses.
	m_velocity += m_impulse;
	
	// Clamp X/Z velocity.
	const Scalar maxVelocity(m_data->getMaxVelocity());
	const Scalar currentVelocity = (m_velocity * c_101).length();
	if (currentVelocity > maxVelocity)
		m_velocity *= (c_101 * maxVelocity / currentVelocity + c_010);

	// Damp velocity; only when no user input.
	if (m_impulse.length() < FUZZY_EPSILON)
	{
		if (currentVelocity > FUZZY_EPSILON)
			m_velocity -= ((m_velocity * c_101) / currentVelocity) * Scalar(m_data->getVelocityDamping()) * dT;
		else
			m_velocity *= c_010;
	}

	// Add gravity.
	m_velocity += Vector4(0.0f, -9.2f, 0.0f) * dT;

	// Reset user impulses.
	m_impulse = Vector4::zero();

	// Step up.
	float stepUpLength = m_data->getStep();
	if (movement.y() > 0.0f)
		stepUpLength += movement.y();

	if (stepVertical(stepUpLength, position))
	{
		// Head hit; cancel out up motion.
		m_velocity *= c_101;
	}

	// Step forward.
	const Vector4 movementXZ = movement * c_101;
	step(movementXZ, position);

	// Step down, step further down to simulate falling.
	float stepDownLength = m_data->getStep();
	if (movement.y() < 0.0f)
		stepDownLength += -movement.y();

	if (stepVertical(-stepDownLength, position))
	{
		// Foot hit; cancel out up motion.
		m_velocity *= c_101;
		m_grounded = true;
	}
	else
		m_grounded = false;

	const Quaternion rotation = Quaternion::fromEulerAngles(m_headAngle, 0.0f, 0.0f);

	m_bodyWide->setTransform(Transform(
		position,
		rotation
	));
	if (m_owner)
		m_owner->setTransform(Transform(
			position - Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f),
			rotation
		));
}

void CharacterComponent::setHeadAngle(float headAngle)
{
	m_headAngle = headAngle;
}

float CharacterComponent::getHeadAngle() const
{
	return m_headAngle;
}

void CharacterComponent::stop()
{
	m_velocity = Vector4::zero();
}

void CharacterComponent::clear()
{
	m_impulse = Vector4::zero();
}

void CharacterComponent::move(const Vector4& motion, bool vertical)
{
	if (vertical)
		m_impulse += motion;
	else
		m_impulse += motion * c_101;
}

bool CharacterComponent::jump()
{
	if (grounded())
	{
		m_impulse += Scalar(m_data->getJumpImpulse()) * c_010;
		return true;
	}
	else
		return false;
}

bool CharacterComponent::grounded() const
{
	return m_grounded;
}

const Vector4& CharacterComponent::getVelocity() const
{
	return m_velocity;
}

bool CharacterComponent::stepVertical(float motion, Vector4& inoutPosition) const
{
	if (std::abs(motion) <= FUZZY_EPSILON)
		return false;

	const float direction = (motion > 0.0f) ? 1.0f : -1.0f;
	bool anyCollision = false;
	QueryResult result;

	if (m_physicsManager->querySweep(
		m_bodySlim,
		Quaternion::identity(),
		inoutPosition,
		Vector4(0.0f, direction, 0.0f),
		std::abs(motion),
		physics::QueryFilter(m_traceInclude, m_traceIgnore),
		result
	))
	{
		inoutPosition += Vector4(0.0f, motion * result.fraction, 0.0f);
		anyCollision = true;
	}
	else
	{
		inoutPosition += Vector4(0.0f, motion, 0.0f);
	}

	return anyCollision;
}

bool CharacterComponent::step(Vector4 motion, Vector4& inoutPosition) const
{
	const Scalar totalMotionLength = motion.length();
	if (totalMotionLength <= FUZZY_EPSILON)
		return false;

	motion /= totalMotionLength;

	bool anyCollision = false;
	QueryResult result;

	Scalar motionLength = totalMotionLength;
	for (int32_t i = 0; i < 8 && motionLength > FUZZY_EPSILON; ++i)
	{
		if (m_physicsManager->querySweep(
			m_bodyWide,
			Quaternion::identity(),
			inoutPosition,
			motion,
			motionLength,
			physics::QueryFilter(m_traceInclude, m_traceIgnore),
			result
		))
		{
			Scalar move = Scalar(motionLength * result.fraction);

			// Don't move entire distance to prevent stability issues.
			const Scalar c_fudge = 0.01_simd;
			inoutPosition += motion * ((move > c_fudge) ? move - c_fudge : move);

			// Adjust movement vector.
			const Scalar k = dot3(-motion, result.normal);
			motion += result.normal * k;
			if (motion.normalize() <= FUZZY_EPSILON)
				break;

			motionLength -= move;
			anyCollision = true;
		}
		else
		{
			inoutPosition += motion * motionLength;
			motionLength = 0.0_simd;
		}
	}

	return anyCollision;
}

}
