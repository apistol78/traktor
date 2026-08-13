/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/World/Character/CharacterComponent.h"

#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "World/Entity.h"

namespace traktor::physics
{
namespace
{

const Vector4 c_101(1.0f, 0.0f, 1.0f);
const Vector4 c_010(0.0f, 1.0f, 0.0f);

/*! Distance, in fractions of character radius, ahead of character used when
 * probing if character is standing on the edge of a step.
 */
const float c_stepForwardTest = 0.5f;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CharacterComponent", CharacterComponent, world::IEntityComponent)

CharacterComponent::CharacterComponent(
	PhysicsManager* physicsManager,
	const CharacterComponentData* data,
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
	, m_minGroundNormalY(std::cos(deg2rad(clamp(data->getMaxSlopeAngle(), 0.0f, 90.0f))))
	, m_headAngle(0.0f)
	, m_velocity(Vector4::zero())
	, m_impulse(Vector4::zero())
	, m_groundNormal(Vector4::zero())
	, m_grounded(false)
	, m_enabled(true)
{
}

CharacterComponent::~CharacterComponent()
{
	destroy();
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
		m_headAngle = transform.rotation().toEulerAngles().x();
	}

	if (m_bodyWide)
		m_bodyWide->setOwner(owner);
	if (m_bodySlim)
		m_bodySlim->setOwner(owner);
}

void CharacterComponent::setTransform(const Transform& transform)
{
	m_bodyWide->setTransform(transform * Transform(Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f)));
	m_headAngle = transform.rotation().toEulerAngles().x();
}

Aabb3 CharacterComponent::getBoundingBox() const
{
	return Aabb3();
}

void CharacterComponent::setEnable(bool enable)
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

void CharacterComponent::update(const world::UpdateParams& update)
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

	const Vector4 movement = m_velocity * dT;

	// Step "assist" enable character to climb steps, and to stay attached to ground
	// when walking down steps. Only applied when character is standing on ground as
	// a character in mid air should neither climb steps nor be snapped to ground.
	const float stepHeight = m_grounded ? m_data->getStep() : 0.0f;

	Vector4 movedPosition = position;
	MoveResult moveResult;
	moveStep(movement, stepHeight, stepHeight, movedPosition, moveResult);

	bool grounded = moveResult.footContact && walkable(moveResult.groundNormal);

	// If the step assist ended up placing the character on a surface which is too steep to
	// stand on then it has most likely climbed a wall. As the contact normal is unreliable
	// when standing on the edge of a step we probe the surface a bit further ahead before
	// discarding the movement.
	if (
		moveResult.footContact &&
		!grounded &&
		movedPosition.y() > position.y() + FUZZY_EPSILON)
	{
		const Vector4 movementXZ = movement * c_101;
		const Scalar movementLength = movementXZ.length();

		// Character is standing on the edge of a step if the surface a bit further
		// ahead is flat enough to stand on.
		const bool onStep =
			movementLength > FUZZY_EPSILON &&
			probeGround(
				moveResult.steppedPosition + (movementXZ / movementLength) * Scalar(c_stepForwardTest * m_data->getRadius()),
				m_data->getStep());

		if (onStep)
			grounded = true;
		else
		{
			// Redo the movement without any step assist so character slide along
			// the wall instead of climbing it.
			movedPosition = position;
			moveResult = MoveResult();
			moveStep(movement, 0.0f, stepHeight, movedPosition, moveResult);
			grounded = moveResult.footContact && walkable(moveResult.groundNormal);
		}
	}

	position = movedPosition;

	// Head hit; cancel out up motion.
	if (moveResult.headContact)
		m_velocity *= c_101;

	// Character is only grounded when standing on a surface which isn't too steep;
	// steep surfaces are slid down along as if falling.
	m_groundNormal = moveResult.footContact ? moveResult.groundNormal : Vector4::zero();
	m_grounded = grounded;

	// Foot hit; cancel out down motion.
	if (m_grounded)
		m_velocity *= c_101;

	const Quaternion rotation = Quaternion::fromEulerAngles(m_headAngle, 0.0f, 0.0f);

	m_bodyWide->setTransform(Transform(
		position,
		rotation));
	if (m_owner)
		m_owner->setTransform(Transform(
			position - Vector4(0.0f, m_data->getHeight() / 2.0f, 0.0f),
			rotation));
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

void CharacterComponent::setVelocity(const Vector4& velocity)
{
	m_velocity = velocity;
}

const Vector4& CharacterComponent::getVelocity() const
{
	return m_velocity;
}

bool CharacterComponent::walkable(const Vector4& normal) const
{
	return normal.y() >= Scalar(m_minGroundNormalY - FUZZY_EPSILON);
}

void CharacterComponent::moveStep(const Vector4& movement, float stepUpHeight, float stepDownHeight, Vector4& inoutPosition, MoveResult& outResult) const
{
	Vector4 normal;

	// Step up.
	float stepUpLength = stepUpHeight;
	if (movement.y() > 0.0f)
		stepUpLength += movement.y();

	if (stepVertical(stepUpLength, inoutPosition, normal))
		outResult.headContact = true;

	// Step forward.
	const Vector4 movementXZ = movement * c_101;
	if (movementXZ.length() > FUZZY_EPSILON)
		step(movementXZ, inoutPosition);

	outResult.steppedPosition = inoutPosition;

	// Fall; slide along surfaces which are too steep to stand on.
	if (movement.y() < 0.0f)
	{
		if (stepFall(-movement.y(), inoutPosition, normal))
		{
			outResult.footContact = true;
			outResult.groundNormal = normal;
		}
	}

	// Step down, so character is able to walk down steps without leaving ground.
	if (stepDownHeight > 0.0f && stepVertical(-stepDownHeight, inoutPosition, normal))
	{
		outResult.footContact = true;
		outResult.groundNormal = normal;
	}
}

bool CharacterComponent::probeGround(Vector4 position, float distance) const
{
	Vector4 normal;
	if (!stepVertical(-distance, position, normal))
		return false;

	return walkable(normal);
}

bool CharacterComponent::stepVertical(float motion, Vector4& inoutPosition, Vector4& outNormal) const
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
		outNormal = result.normal;
		anyCollision = true;
	}
	else
	{
		inoutPosition += Vector4(0.0f, motion, 0.0f);
	}

	return anyCollision;
}

bool CharacterComponent::stepFall(float motion, Vector4& inoutPosition, Vector4& outNormal) const
{
	if (motion <= FUZZY_EPSILON)
		return false;

	Vector4 direction = -c_010;
	Scalar motionLength(motion);

	bool anyCollision = false;
	QueryResult result;

	for (int32_t i = 0; i < 4 && motionLength > FUZZY_EPSILON; ++i)
	{
		if (!m_physicsManager->querySweep(
				m_bodySlim,
				Quaternion::identity(),
				inoutPosition,
				direction,
				motionLength,
				physics::QueryFilter(m_traceInclude, m_traceIgnore),
				result))
		{
			inoutPosition += direction * motionLength;
			break;
		}

		const Scalar move = motionLength * Scalar(result.fraction);
		inoutPosition += direction * move;

		outNormal = result.normal;
		anyCollision = true;

		// Landed on a surface which is flat enough to stand on.
		if (walkable(result.normal))
			break;

		// Surface is too steep to stand on; continue falling along the surface, using
		// same over compensation as when stepping in order to not get stuck.
		const Scalar k = abs(dot3(-direction, result.normal)) * 1.01_simd;
		direction += result.normal * k;
		if (direction.normalize() <= FUZZY_EPSILON)
			break;

		motionLength -= move + 0.01_simd;
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

			// Surfaces which are too steep to walk on are treated as vertical walls,
			// i.e. character slide along them instead of being pushed up.
			Vector4 normal = result.normal;
			if (!walkable(normal))
			{
				normal *= c_101;
				if (normal.length() <= FUZZY_EPSILON)
					break;
				normal = normal.normalized();
			}

			// Adjust movement vector; this contain a couple of magic constants
			// which is to compensate for unwanted jitter when unable to solve movement.
			const Scalar k = abs(dot3(-motion, normal)) * 1.01_simd;
			motion += normal * k;
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
