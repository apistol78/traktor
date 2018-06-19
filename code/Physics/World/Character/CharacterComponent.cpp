/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "World/Entity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CharacterComponent", CharacterComponent, world::IEntityComponent)

CharacterComponent::CharacterComponent(
	PhysicsManager* physicsManager,
	const CharacterComponentData* data,
	Body* body,
	uint32_t traceInclude,
	uint32_t traceIgnore
)
:	m_owner(0)
,	m_physicsManager(physicsManager)
,	m_data(data)
,	m_body(body)
,	m_traceInclude(traceInclude)
,	m_traceIgnore(traceIgnore)
,	m_headAngle(0.0f)
,	m_velocity(Vector4::zero())
,	m_grounded(false)
{
}

void CharacterComponent::destroy()
{
	safeDestroy(m_body);
	m_owner = 0;
}

void CharacterComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != 0)
	{
		Transform transform;
		if (m_owner->getTransform(transform))
		{
			m_body->setTransform(transform);
			m_body->setEnable(true);
		}
	}
}

void CharacterComponent::setTransform(const Transform& transform)
{
	m_body->setTransform(transform);
}

Aabb3 CharacterComponent::getBoundingBox() const
{
	return Aabb3();
}

void CharacterComponent::update(const world::UpdateParams& update)
{
	float dT = update.deltaTime;

	Quaternion rotation = Quaternion::fromEulerAngles(m_headAngle, 0.0f, 0.0f);
	Vector4 movement = m_velocity * Scalar(dT);
	Vector4 position = m_body->getTransform().translation();
	QueryResult result;

	// Integrate velocity.
	m_velocity += Vector4(0.0f, -9.2f, 0.0f) * Scalar(dT);

	// Step up.
	const Vector4 stepUpDir(0.0f, 1.0f, 0.0f);
	
	float stepUpLength = m_data->getStepHeight();
	if (movement.y() > 0.0f)
		stepUpLength += movement.y();

	if (m_physicsManager->querySweep(
		m_body,
		rotation,
		position,
		stepUpDir,
		stepUpLength,
		physics::QueryFilter(m_traceInclude, m_traceIgnore),
		result
	))
	{
		position = position + stepUpDir * Scalar(stepUpLength * result.fraction);
		
		// Head hit; cancel out up motion.
		m_velocity *= Vector4(1.0f, 0.0f, 1.0f, 0.0f);
	}
	else
		position = position + stepUpDir * Scalar(stepUpLength);

	// Step forward.
	Vector4 movementXZ = movement * Vector4(1.0f, 0.0f, 1.0f);
	Scalar totalMovementLength = movementXZ.normalize();
	if (totalMovementLength > FUZZY_EPSILON)
	{
		Scalar movementLength = totalMovementLength;
		for (int32_t i = 0; i < 10 && movementLength > FUZZY_EPSILON; ++i)
		{
			if (m_physicsManager->querySweep(
				m_body,
				rotation,
				position,
				movementXZ,
				movementLength,
				physics::QueryFilter(m_traceInclude, m_traceIgnore),
				result
			))
			{
				Scalar move = Scalar(movementLength * result.fraction);

				// Don't move entire distance to prevent stability issues.
				const Scalar c_fudge(0.01f);
				if (move > c_fudge)
					move -= c_fudge;

				position = position + movement * move;
				
				// Adjust movement vector.
				Scalar k = dot3(-movementXZ, result.normal);
				movementXZ += result.normal * k;
				movementXZ *= Vector4(1.0f, 0.0f, 1.0f);
				if (movementXZ.normalize() <= FUZZY_EPSILON)
					break;

				movementLength -= move;
			}
			else
			{
				position = position + movementXZ * movementLength;
				movementLength = Scalar(0.0f);
			}
		}
	}

	// Step down, step further down to simulate falling.
	const Vector4 stepDownDir(0.0f, -1.0f, 0.0f);

	float stepDownLength = m_data->getStepHeight();
	if (movement.y() < 0.0f)
		stepDownLength += -movement.y();

	if (m_physicsManager->querySweep(
		m_body,
		rotation,
		position,
		stepDownDir,
		stepDownLength,
		physics::QueryFilter(m_traceInclude, m_traceIgnore),
		result
	))
	{
		position = position + stepDownDir * Scalar(stepDownLength * result.fraction);

		// Foot hit; cancel out up motion.
		m_velocity *= Vector4(1.0f, 0.0f, 1.0f, 0.0f);

		m_grounded = true;
	}
	else
	{
		position = position + stepDownDir * Scalar(stepDownLength);
		m_grounded = false;
	}

	m_body->setTransform(Transform(
		position,
		rotation
	));

	if (m_owner)
		m_owner->setTransform(Transform(
			position,
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

void CharacterComponent::setVelocity(const Vector4& velocity)
{
	m_velocity = velocity;
}

const Vector4& CharacterComponent::getVelocity() const
{
	return m_velocity;
}

bool CharacterComponent::isGrounded() const
{
	return m_grounded;
}

	}
}
