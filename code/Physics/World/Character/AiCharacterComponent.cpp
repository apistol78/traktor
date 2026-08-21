/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/World/Character/AiCharacterComponent.h"

#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/AiCharacterComponentData.h"
#include "World/Entity.h"

#include <cmath>

namespace traktor::physics
{
namespace
{

const Vector4 c_101(1.0f, 0.0f, 1.0f);
const Vector4 c_010(0.0f, 1.0f, 0.0f);

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.AiCharacterComponent", AiCharacterComponent, world::IEntityComponent)

AiCharacterComponent::AiCharacterComponent(
	PhysicsManager* physicsManager,
	const AiCharacterComponentData* data,
	Body* bodyWide,
	Body* bodySlim,
	uint32_t traceInclude,
	uint32_t traceIgnore)
	: m_owner(nullptr)
	, m_physicsManager(physicsManager)
	, m_data(data)
	, m_bodyWide(bodyWide)
	, m_bodySlim(bodySlim)
	, m_traceInclude(traceInclude)
	, m_traceIgnore(traceIgnore)
	, m_maxVelocity(data->getMaxVelocity())
	, m_headAngle(0.0f)
	, m_velocity(Vector4::zero())
	, m_impulse(Vector4::zero())
	, m_grounded(false)
	, m_enabled(true)
{
}

AiCharacterComponent::~AiCharacterComponent()
{
	destroy();
}

void AiCharacterComponent::destroy()
{
	safeDestroy(m_bodyWide);
	safeDestroy(m_bodySlim);
	m_owner = nullptr;
}

void AiCharacterComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		const Transform transform = m_owner->getTransform();
		m_bodyWide->setTransform(transform * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
		m_bodyWide->setEnable(true);
		m_headAngle = transform.rotation().toEulerAngles().x();
	}

	if (m_bodyWide)
		m_bodyWide->setOwner(owner);
	if (m_bodySlim)
		m_bodySlim->setOwner(owner);
}

void AiCharacterComponent::setTransform(const Transform& transform)
{
	m_bodyWide->setTransform(transform * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
	m_headAngle = transform.rotation().toEulerAngles().x();
}

Aabb3 AiCharacterComponent::getBoundingBox() const
{
	return Aabb3();
}

void AiCharacterComponent::setEnable(bool enable)
{
	if (enable == m_enabled)
		return;

	m_enabled = enable;

	// Add/remove the collision body so a disabled controller neither collides
	// nor is simulated.
	if (m_bodyWide)
		m_bodyWide->setEnable(enable);

	if (enable)
	{
		// Re-sync the collision body to the owner after having been disabled.
		if (m_owner)
			m_bodyWide->setTransform(m_owner->getTransform() * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
	}
	else
	{
		// Stop dead so nothing lingers when the controller is re-enabled.
		m_velocity = Vector4::zero();
		m_impulse = Vector4::zero();
	}
}

void AiCharacterComponent::update(const world::UpdateParams& update)
{
	// A disabled controller does not simulate nor drive the owner transform,
	// leaving the entity free to be positioned by another system (e.g. rag doll).
	if (!m_enabled)
		return;

	const Scalar dT(update.deltaTime);
	Vector4 position = m_bodyWide->getTransform().translation();

	// Add user impulses.
	m_velocity += m_impulse;

	// Clamp X/Z velocity.
	const Scalar maxVelocity(m_maxVelocity);
	const Scalar currentVelocity = (m_velocity * c_101).length();
	if (currentVelocity > maxVelocity)
		m_velocity *= (c_101 * maxVelocity / currentVelocity + c_010);

	// Damp velocity; only when no user input.
	if (m_impulse.length() < FUZZY_EPSILON)
	{
		if (currentVelocity > FUZZY_EPSILON)
			m_velocity -= (m_velocity * c_101) * Scalar(m_data->getVelocityDamping());
		else
			m_velocity *= c_010;
	}

	// Add gravity.
	m_velocity += Vector4(0.0f, -9.2f, 0.0f) * dT;

	// Reset user impulses.
	m_impulse = Vector4::zero();

	// Step up.
	const Vector4 movement = m_velocity * dT;
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
	if (movementXZ.length() > FUZZY_EPSILON)
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

	// Carried rather than teleported, so the capsule has a velocity while it walks and
	// hands momentum to the pushable bodies it leans on -- a door shoved this way keeps
	// swinging after the character stops, instead of only being nudged aside as long as
	// it is being penetrated.
	m_bodyWide->moveKinematic(Transform(
		position,
		rotation), update.deltaTime);
	if (m_owner)
		m_owner->setTransform(Transform(
			position - Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f),
			rotation));
}

void AiCharacterComponent::setHeadAngle(float headAngle)
{
	m_headAngle = headAngle;
}

float AiCharacterComponent::getHeadAngle() const
{
	return m_headAngle;
}

void AiCharacterComponent::stop()
{
	m_velocity = Vector4::zero();
}

void AiCharacterComponent::clear()
{
	m_impulse = Vector4::zero();
}

void AiCharacterComponent::move(const Vector4& motion, bool vertical)
{
	if (vertical)
		m_impulse += motion;
	else
		m_impulse += motion * c_101;
}

bool AiCharacterComponent::jump()
{
	if (grounded())
	{
		m_impulse += Scalar(m_data->getJumpImpulse()) * c_010;
		return true;
	}
	else
		return false;
}

bool AiCharacterComponent::grounded() const
{
	return m_grounded;
}

void AiCharacterComponent::setVelocity(const Vector4& velocity)
{
	m_velocity = velocity;
}

const Vector4& AiCharacterComponent::getVelocity() const
{
	return m_velocity;
}

bool AiCharacterComponent::stepVertical(float motion, Vector4& inoutPosition) const
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
			result))
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

bool AiCharacterComponent::step(Vector4 motion, Vector4& inoutPosition) const
{
	const Scalar totalMotionLength = motion.length();
	if (totalMotionLength <= FUZZY_EPSILON)
		return false;

	motion /= totalMotionLength;

	bool anyCollision = false;
	QueryResult result;

	Scalar motionLength = totalMotionLength;
	for (int32_t i = 0; i < 16 && motionLength > FUZZY_EPSILON; ++i)
	{
		if (m_physicsManager->querySweep(
				m_bodyWide,
				Quaternion::identity(),
				inoutPosition,
				motion,
				motionLength,
				physics::QueryFilter(m_traceInclude, m_traceIgnore),
				result))
		{
			const Scalar move = Scalar(motionLength * result.fraction);
			inoutPosition += motion * move;

			// Adjust movement vector; this contain a couple of magic constants
			// which is to compensate for unwanted jitter when unable to solve movement.
			const Scalar k = abs(dot3(-motion, result.normal)) * 1.01_simd;
			motion += result.normal * k;
			if (motion.normalize() <= FUZZY_EPSILON)
				break;

			motionLength -= move + 0.01_simd;
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
