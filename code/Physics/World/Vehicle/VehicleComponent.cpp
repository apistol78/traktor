/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"
#include "World/Entity.h"

#include "Core/Log/Log.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const float c_maxSuspensionForce = 250.0f;
const float c_maxDampingForce = 250.0f;
const Scalar c_slowGripCoeff = 1.0_simd;
const Scalar c_fastGripCoeff = 0.01_simd;
const float c_throttleThreshold = 0.01f;
const Scalar c_linearVelocityThreshold = 4.0_simd;
const float c_suspensionTraceRadius = 0.25f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.VehicleComponent", VehicleComponent, world::IEntityComponent)

VehicleComponent::VehicleComponent(
	PhysicsManager* physicsManager,
	const VehicleComponentData* data,
	const RefArray< Wheel >& wheels,
	uint32_t traceInclude,
	uint32_t traceIgnore
)
:	m_owner(nullptr)
,	m_physicsManager(physicsManager)
,	m_data(data)
,	m_wheels(wheels)
,	m_traceInclude(traceInclude)
,	m_traceIgnore(traceIgnore)
,	m_steerAngle(0.0f)
,	m_steerAngleTarget(0.0f)
,	m_engineThrottle(0.0f)
,	m_breaking(0.0f)
,	m_airBorn(true)
{
}

void VehicleComponent::destroy()
{
	m_owner = nullptr;
}

void VehicleComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void VehicleComponent::setTransform(const Transform& transform)
{
}

Aabb3 VehicleComponent::getBoundingBox() const
{
	return Aabb3();
}

void VehicleComponent::update(const world::UpdateParams& update)
{
	T_ASSERT(m_owner != nullptr);

	RigidBodyComponent* bodyComponent = m_owner->getComponent< RigidBodyComponent >();
	if (!bodyComponent)
		return;

	Body* body = bodyComponent->getBody();
	if (!body)
		return;

	const float dT = update.deltaTime;

	updateSteering(body, dT);
	updateSuspension(body, dT);
	updateFriction(body, dT);
	updateEngine(body, dT);
	updateWheels(body, dT);
}

void VehicleComponent::setSteerAngle(float steerAngle)
{
	m_steerAngleTarget = steerAngle;
}

float VehicleComponent::getSteerAngle() const
{
	return m_steerAngleTarget;
}

float VehicleComponent::getSteerAngleFiltered() const
{
	return m_steerAngle;
}

void VehicleComponent::setEngineThrottle(float engineThrottle)
{
	m_engineThrottle = engineThrottle;
}

float VehicleComponent::getEngineThrottle() const
{
	return m_engineThrottle;
}

void VehicleComponent::setBreaking(float breaking)
{
	m_breaking = breaking;
}

float VehicleComponent::getBreaking() const
{
	return m_breaking;
}

void VehicleComponent::updateSteering(Body* body, float dT)
{
	// Update steer angle, aiming for target angle.
	if (m_steerAngle < m_steerAngleTarget)
	{
		const float dA = min(m_steerAngleTarget - m_steerAngle, m_data->getSteerAngleVelocity() * dT);
		m_steerAngle += dA;
	}
	else
	{
		const float dA = min(m_steerAngle - m_steerAngleTarget, m_data->getSteerAngleVelocity() * dT);
		m_steerAngle -= dA;
	}

	// Update wheel direction from steering.
	const Vector4 direction(std::sin(m_steerAngle), 0.0f, std::cos(m_steerAngle), 0.0f);
	const Vector4 directionPerp(std::cos(m_steerAngle), 0.0f, -std::sin(m_steerAngle), 0.0f);
	for (auto wheel : m_wheels)
	{
		if (wheel->data->getSteer())
		{
			wheel->direction = direction;
			wheel->directionPerp = directionPerp;
		}
	}
}

void VehicleComponent::updateSuspension(Body* body, float dT)
{
	physics::QueryResult result;

	const Transform bodyT = body->getTransform();
	const Transform bodyTinv = bodyT.inverse();

	m_airBorn = true;

	for (auto wheel : m_wheels)
	{
		const WheelData* data = wheel->data;
		T_ASSERT(data != nullptr);

		const Vector4 anchorW = bodyT * data->getAnchor().xyz1();
		const Vector4 axisW = bodyT * -data->getAxis().xyz0().normalized();

		float contactFudge = 0.0f;

		if (m_physicsManager->queryRay(
			anchorW,
			axisW,
			data->getSuspensionLength().max + data->getRadius() + m_data->getFudgeDistance(),
			physics::QueryFilter(m_traceInclude, m_traceIgnore),
			false,
			result
		))
		{
			if (result.distance <= data->getSuspensionLength().max + data->getRadius())
				contactFudge = 1.0f;
			else
				contactFudge = 1.0f - (result.distance - (data->getSuspensionLength().max + data->getRadius())) / m_data->getFudgeDistance();
		}

		if (contactFudge >= FUZZY_EPSILON)
		{
			const Vector4 normal = result.normal.normalized();

			// Suspension current length.
			float suspensionLength = result.distance - data->getRadius();

			// Clamp lengths.
			if (suspensionLength < data->getSuspensionLength().min)
				suspensionLength = data->getSuspensionLength().min;
			else if (suspensionLength > data->getSuspensionLength().max)
				suspensionLength = data->getSuspensionLength().max;

			// Suspension velocity.
			const float suspensionVelocity = (wheel->suspensionLength - suspensionLength) / dT;

			// Suspension forces.
			const float t = 1.0f - (suspensionLength - data->getSuspensionLength().min) / (data->getSuspensionLength().max - data->getSuspensionLength().min);
			const float springForce = clamp(t * data->getSuspensionSpring(), 0.0f, c_maxSuspensionForce);
			const float dampingForce = clamp(suspensionVelocity * data->getSuspensionDamping(), -c_maxDampingForce, c_maxDampingForce);

			// Apply forces.
			body->addForceAt(
				anchorW,
				normal * Scalar(springForce + dampingForce),
				false
			);

			// Apply sway-bar force on the opposite side.
			body->addForceAt(
				bodyT * (data->getAnchor() * Vector4(-1.0f, 1.0f, 1.0f, 1.0f)),
				normal * -Scalar((springForce + dampingForce) * m_data->getSwayBarForce()),
				false
			);

			// Save suspension state.
			wheel->suspensionLength = suspensionLength;

			// Contact attributes.
			Vector4 contactVelocity;
			if (!wheel->contact)
			{
				// If no previous contact then we estimate velocity by projecting onto ground.
				const Vector4 wheelVelocity = body->getVelocityAt(result.position.xyz1(), false);
				const Vector4 groundVelocity = result.body->getVelocityAt(result.position.xyz1(), false);
				const Vector4 velocity = wheelVelocity - groundVelocity;
				const Scalar k = dot3(normal, velocity);
				contactVelocity = velocity - normal * (-k);
			}
			else
			{
				// Calculate explicit velocity based on contact movement.
				const Vector4 groundVelocity = result.body->getVelocityAt(result.position.xyz1(), false);
				const Vector4 contactMovement = (result.position - wheel->contactPosition - groundVelocity * Scalar(dT)).xyz0();
				contactVelocity = contactMovement / Scalar(dT);
			}
			
			wheel->contact = true;
			wheel->contactFudge = contactFudge;
			wheel->contactMaterial = result.material;
			wheel->contactPosition = result.position.xyz1();
			wheel->contactNormal = normal;
			wheel->contactVelocity = contactVelocity;

			m_airBorn = false;
		}
		else
		{
			wheel->suspensionLength = data->getSuspensionLength().max;

			wheel->contact = false;
			wheel->contactFudge = 0.0f;
			wheel->contactMaterial = 0;
			wheel->contactPosition = Vector4::origo();
			wheel->contactNormal = Vector4::zero();
			wheel->contactVelocity = Vector4::zero();
		}
	}
}

void VehicleComponent::updateFriction(Body* body, float dT)
{
	const Transform bodyT = body->getTransform();
	const Transform bodyTinv = bodyT.inverse();

	Scalar rollingFriction = 0.0_simd;
	const Scalar totalMass = 1.0_simd / Scalar(body->getInverseMass());
	const Scalar massPerWheel = totalMass / Scalar(m_wheels.size());
	const Scalar breakingForce(m_data->getBreakingForce());

	for (auto wheel : m_wheels)
	{
		wheel->sliding = false;

		// Do not apply friction if wheel is not in contact with ground.
		if (!wheel->contact)
			continue;

		const WheelData* data = wheel->data;
		T_ASSERT(data != nullptr);

		// Get suspension axis in world space.
		const Vector4 axisW = bodyT * data->getAxis();

		// Wheel directions in world space.
		Vector4 directionW = bodyT * wheel->direction;
		Vector4 directionPerpW = bodyT * wheel->directionPerp;

#if 0
		// Project wheel directions onto contact plane.
		directionW -= wheel->contactNormal * dot3(wheel->contactNormal, directionW);
		directionPerpW -= wheel->contactNormal * dot3(wheel->contactNormal, directionPerpW);
#endif
		directionW = directionW.normalized();
		directionPerpW = directionPerpW.normalized();

		// Calculate grip.
		Scalar grip = 1.0_simd;
		grip *= abs(dot3(axisW, wheel->contactNormal));
		grip *= Scalar(wheel->contactFudge);

		// Determine velocities and percent of maximum velocity.
		const Scalar forwardVelocity = dot3(directionW, wheel->contactVelocity);
		const Scalar sideVelocity = dot3(directionPerpW, wheel->contactVelocity);

		// Method factor lerps between slip angle based friction and purely perpendicular friction.
		const Scalar c_methodLimit = 4.0_simd;
		const Scalar method = clamp(1.0_simd - abs(forwardVelocity) / c_methodLimit, 0.0_simd, 1.0_simd);

		// Calculate slip angle.
		const float k = std::atan2(sideVelocity, forwardVelocity);
		const float slipAngle = abs(k);

		// Calculate amount of force from slip angle. \fixme Should use curves.
		const float peakSlipFriction = data->getSlipCornerForce();
		const float maxSlipAngle = data->getPeakSlipAngle();

		float force = 0.0f;
		if (slipAngle < maxSlipAngle)
			force = (slipAngle / maxSlipAngle) * peakSlipFriction;
		else
		{
			const float c_fallOff = 2.0f;
			const float f = clamp(rad2deg(slipAngle - maxSlipAngle) / c_fallOff, 0.0f, 1.0f);
			force = peakSlipFriction * f;

			// Do not tag sliding if going too slow.
			if (abs(forwardVelocity) > 1.0f)
				wheel->sliding = true;
		}

		// Apply friction force.
		body->addForceAt(
			wheel->contactPosition,
			directionPerpW * Scalar(force * sign(-sideVelocity)) * grip * (1.0_simd - method),
			false
		);
	
		// Apply perpendicular friction force if going slow.
		body->addForceAt(
			wheel->contactPosition,
			directionPerpW * -sideVelocity * Scalar(peakSlipFriction) * grip * method,
			false
		);

		// Accumulate rolling friction, applied at center of mass for simplicity.
		rollingFriction += forwardVelocity * Scalar(data->getRollingFriction()) * grip;

		// Calculate breaking force.
		if (m_breaking > FUZZY_EPSILON)
		{
			const Scalar f = Scalar(m_breaking * data->getBreakFactor());
			const Scalar mag = sign(forwardVelocity) * breakingForce * f * grip;
			body->addForceAt(
				wheel->contactPosition,
				directionW * -mag,
				false
			);
		}
	}

	if (abs(rollingFriction) > FUZZY_EPSILON)
	{
		body->addForceAt(
			Vector4::origo(),
			Vector4(0.0f, 0.0f, -rollingFriction, 0.0f),
			true
		);
	}
}

void VehicleComponent::updateEngine(Body* body, float /*dT*/)
{
	const Transform bodyT = body->getTransform();
	const Transform bodyTinv = bodyT.inverse();

	const Scalar forwardVelocity = dot3(body->getLinearVelocity(), bodyT.axisZ());
	const Scalar engineForce = Scalar(m_engineThrottle * m_data->getEngineForce()) * (1.0_simd - clamp(abs(forwardVelocity) / Scalar(m_data->getMaxVelocity()), 0.0_simd, 1.0_simd));

	for (auto wheel : m_wheels)
	{
		if (!wheel->contact)
			continue;

		const WheelData* data = wheel->data;
		T_ASSERT(data != nullptr);

		if (!data->getDrive())
			continue;

		const Vector4 direction = (wheel->direction * Vector4(1.0f, 0.0f, 1.0f, 0.0f)).normalized();
		const Scalar grip = clamp(wheel->contactNormal.y(), 0.0_simd, 1.0_simd) * Scalar(wheel->contactFudge);

		body->addForceAt(
			bodyTinv * wheel->contactPosition,
			direction * engineForce * grip,
			true
		);
	}
}

void VehicleComponent::updateWheels(Body* body, float dT)
{
	const Transform bodyT = body->getTransform();

	for (auto wheel : m_wheels)
	{
		float targetVelocity = 0.0f;

		const WheelData* data = wheel->data;
		T_ASSERT(data != nullptr);

		if (!data->getDrive() || abs(m_engineThrottle) <= FUZZY_EPSILON)
			targetVelocity = wheel->velocity * 0.95f;
		else
			targetVelocity = (m_engineThrottle * m_data->getMaxVelocity()) / data->getRadius();

		if (wheel->contact)
		{
			float d = dot3(wheel->contactVelocity, bodyT * wheel->direction);
			wheel->velocity = lerp(d / data->getRadius(), targetVelocity, 0.25f);
		}
		else
			wheel->velocity = targetVelocity;

		wheel->angle += wheel->velocity * dT;
	}
}

	}
}
