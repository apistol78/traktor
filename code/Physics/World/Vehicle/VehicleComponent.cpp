/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"
#include "World/Entity.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{
		
const float c_maxSuspensionForce = 250.0f;
const float c_maxDampingForce = 250.0f;
const float c_slowGripCoeff = 0.9f;
const float c_fastGripCoeff = 0.6f;
const float c_throttleThreshold = 0.01f;
const float c_linearVelocityThreshold = 4.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.VehicleComponent", VehicleComponent, world::IEntityComponent)

VehicleComponent::VehicleComponent(
	PhysicsManager* physicsManager,
	const VehicleComponentData* data,
	Body* body,
	const RefArray< Wheel >& wheels,
	uint32_t traceInclude,
	uint32_t traceIgnore
)
:	m_owner(0)
,	m_physicsManager(physicsManager)
,	m_data(data)
,	m_body(body)
,	m_wheels(wheels)
,	m_traceInclude(traceInclude)
,	m_traceIgnore(traceIgnore)
,	m_totalMass(0.0f)
,	m_steerAngle(0.0f)
,	m_steerAngleTarget(0.0f)
,	m_engineThrottle(0.0f)
,	m_airBorn(true)
{
	m_totalMass = 1.0f / m_body->getInverseMass();
}

void VehicleComponent::destroy()
{
	safeDestroy(m_body);
	m_owner = 0;
}

void VehicleComponent::setOwner(world::Entity* owner)
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

void VehicleComponent::setTransform(const Transform& transform)
{
	m_body->setTransform(transform);
}

Aabb3 VehicleComponent::getBoundingBox() const
{
	return Aabb3();
}

void VehicleComponent::update(const world::UpdateParams& update)
{
	if (m_owner)
		m_owner->setTransform(m_body->getTransform());

	float dT = update.deltaTime;

	updateSteering(dT);
	updateSuspension(dT);
	updateFriction(dT);
	updateEngine(dT);
	updateWheels(dT);
}

void VehicleComponent::setSteerAngle(float steerAngle)
{
	m_steerAngleTarget = steerAngle;
}

void VehicleComponent::setEngineThrottle(float engineThrottle)
{
	m_engineThrottle = engineThrottle;
}

void VehicleComponent::updateSteering(float dT)
{
	if (m_steerAngle < m_steerAngleTarget)
	{
		float dA = min(m_steerAngleTarget - m_steerAngle, m_data->getSteerAngleVelocity() * dT);
		m_steerAngle += dA;
	}
	else
	{
		float dA = min(m_steerAngle - m_steerAngleTarget, m_data->getSteerAngleVelocity() * dT);
		m_steerAngle -= dA;
	}

	for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	{
		if ((*i)->data->getSteer())
		{
			(*i)->direction = Vector4(std::sin(m_steerAngle), 0.0f, std::cos(m_steerAngle), 0.0f);
			(*i)->directionPerp = Vector4(std::cos(m_steerAngle), 0.0f, -std::sin(m_steerAngle), 0.0f);
		}
	}
}

void VehicleComponent::updateSuspension(float dT)
{
	physics::QueryResult result;

	Transform bodyT = m_body->getTransform();
	Transform bodyTinv = bodyT.inverse();

	m_airBorn = true;

	for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	{
		const WheelData* data = (*i)->data;
		T_ASSERT (data != 0);

		Vector4 anchorW = bodyT * data->getAnchor().xyz1();
		Vector4 axisW = bodyT * -data->getAxis().xyz0().normalized();

		const float c_suspensionTraceRadius = 0.25f;
		float contactFudge = 0.0f;

		if (m_physicsManager->querySweep(
			anchorW,
			axisW,
			data->getSuspensionLength().max + data->getRadius() + m_data->getFudgeDistance(),
			c_suspensionTraceRadius,
			physics::QueryFilter(m_traceInclude, m_traceIgnore),
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
			Vector4 normal = result.normal.normalized();

			// Suspension current length.
			float suspensionLength = result.distance - data->getRadius();

			// Clamp lengths.
			if (suspensionLength < data->getSuspensionLength().min)
				suspensionLength = data->getSuspensionLength().min;
			else if (suspensionLength > data->getSuspensionLength().max)
				suspensionLength = data->getSuspensionLength().max;
	
			// Suspension velocity.
			float suspensionVelocity = ((*i)->suspensionPreviousLength - suspensionLength) / dT;
	
			// Suspension forces.
			float t = 1.0f - (suspensionLength - data->getSuspensionLength().min) / (data->getSuspensionLength().max - data->getSuspensionLength().min);

			float springForce = clamp(t * data->getSuspensionSpringCoeff(), -c_maxSuspensionForce, c_maxSuspensionForce);
			float dampingForce = clamp(suspensionVelocity * data->getSuspensionDampingCoeff(), -c_maxDampingForce, c_maxDampingForce);

			// Apply forces.
			m_body->addForceAt(
				anchorW,
				normal * Scalar(springForce + dampingForce),
				false
			);

			// Apply sway-bar force on the opposite side.
			m_body->addForceAt(
				bodyT * (data->getAnchor() * Vector4(-1.0f, 1.0f, 1.0f, 1.0f)),
				normal * -Scalar((springForce + dampingForce) * m_data->getSwayBarForceCoeff()),
				false
			);

			// Save suspension state.
			(*i)->suspensionPreviousLength = suspensionLength;

			// Contact attributes.
			Vector4 wheelVelocity = m_body->getVelocityAt(result.position.xyz1(), false);
			Vector4 groundVelocity = result.body->getVelocityAt(result.position.xyz1(), false);
			Vector4 velocity = wheelVelocity - groundVelocity;

			(*i)->contact = true;
			(*i)->contactFudge = contactFudge;
			(*i)->contactMaterial = result.material;
			(*i)->contactPosition = result.position.xyz1();
			(*i)->contactNormal = normal;

			Scalar k = dot3(normal, velocity);
			(*i)->contactVelocity = velocity - normal * (-k);

			m_airBorn = false;
		}
		else
		{
			// Save suspension state.
			(*i)->suspensionPreviousLength = data->getSuspensionLength().max;

			(*i)->contact = false;
			(*i)->contactFudge = 0.0f;
			(*i)->contactMaterial = 0;
			(*i)->contactPosition = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			(*i)->contactNormal = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			(*i)->contactVelocity = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		(*i)->suspensionFilteredLength = (*i)->suspensionPreviousLength * 0.2f + (*i)->suspensionFilteredLength * 0.8f;
	}
}

void VehicleComponent::updateFriction(float /*dT*/)
{
	Transform bodyT = m_body->getTransform();
	Transform bodyTinv = bodyT.inverse();

	Scalar rollingFriction(0.0f);
	
	//float totalGrip = 0.0f;
	//for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	//{
	//	if (!(*i)->contact)
	//		continue;

	//	Vector4 axis = bodyT * (*i)->data->getAxis();
	//	totalGrip += dot3(axis, (*i)->contactNormal) * Scalar((*i)->contactFudge);
	//}

	//totalGrip /= float(m_wheels.size());
	//float frictionPower = std::pow(clamp(totalGrip, 0.0f, 1.0f), 4.0f);

	float massPerWheel = m_totalMass / m_wheels.size();

	for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	{
		if (!(*i)->contact)
			continue;

		const WheelData* data = (*i)->data;
		T_ASSERT (data != 0);

		Vector4 axis = bodyT * data->getAxis();
		
		Vector4 directionW = bodyT * (*i)->direction;
		Vector4 directionPerpW = bodyT * (*i)->directionPerp;

		directionW -= (*i)->contactNormal * dot3((*i)->contactNormal, directionW);
		directionPerpW -= (*i)->contactNormal * dot3((*i)->contactNormal, directionPerpW);

		directionW = directionW.normalized();
		directionPerpW = directionPerpW.normalized();

		Scalar forwardVelocity = dot3(directionW, (*i)->contactVelocity);
		Scalar sideVelocity = dot3(directionPerpW, (*i)->contactVelocity);
		
		//sideVelocity = clamp(
		//	sideVelocity,
		//	Scalar(-2.0f),
		//	Scalar(2.0f)
		//);

		//Scalar gripCoeff = lerp(Scalar(c_slowGripCoeff), Scalar(c_fastGripCoeff), clamp(abs(forwardVelocity) / Scalar(m_data->getMaxVelocity()), Scalar(0.0f), Scalar(1.0f)));
		//Scalar grip = dot3(axis, (*i)->contactNormal) * Scalar((*i)->contactFudge) * gripCoeff;

		Scalar grip(0.01f);

		m_body->addImpulse(
			(*i)->contactPosition,
			directionPerpW * -sideVelocity * Scalar(massPerWheel * grip/* * frictionPower*/),
			false
		);

		rollingFriction += forwardVelocity * Scalar(data->getRollingFrictionCoeff()) * grip;
	}

	//if (abs(rollingFriction) > FUZZY_EPSILON)
	//{
	//	m_body->addForceAt(
	//		Vector4::origo(),
	//		Vector4(0.0f, 0.0f, -rollingFriction, 0.0f),
	//		true
	//	);
	//}

	//// Help keep vehicle stationary if almost standstill.
	//if (!m_airBorn)
	//{
	//	Vector4 linearVelocityW = m_body->getLinearVelocity();
	//	Scalar forwardVelocityW = dot3(bodyT.axisZ(), linearVelocityW);

	//	bool throttleIdle = abs(m_engineThrottle) < c_throttleThreshold;
	//	bool almostStill = bool(abs(forwardVelocityW) < c_linearVelocityThreshold);

	//	if (throttleIdle && almostStill)
	//	{
	//		float s = 1.0f - abs(forwardVelocityW) / c_linearVelocityThreshold;
	//		m_body->addLinearImpulse(
	//			-bodyT.axisZ() * forwardVelocityW * Scalar(s * s * 0.3f * m_totalMass),
	//			false
	//		);
	//	}
	//}

	//// Apply some rotational damping to prevent oscillation from suspension forces.
	//if (!m_airBorn && abs(m_steerAngle) < FUZZY_EPSILON)
	//{
	//	const float damping = 0.45f;
	//	Scalar headVelocity = dot3(m_body->getAngularVelocity(), bodyT.axisY());
	//	Matrix33 inertiaInv = m_body->getInertiaTensorInverseWorld();
	//	Vector4 C = Vector4(0.0f, -headVelocity * Scalar(damping), 0.0f);
	//	m_body->addAngularImpulse(inertiaInv.inverse() * (bodyT * C), false);
	//}
}

void VehicleComponent::updateEngine(float /*dT*/)
{
	Transform bodyT = m_body->getTransform();
	Transform bodyTinv = bodyT.inverse();

	Scalar forwardVelocity = dot3(m_body->getLinearVelocity(), bodyT.axisZ());
	Scalar engineForce = Scalar(m_engineThrottle * m_data->getEngineForceCoeff()) * (Scalar(1.0f) - clamp(abs(forwardVelocity) / Scalar(m_data->getMaxVelocity()), Scalar(0.0f), Scalar(1.0f)));

	for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	{
		if (!(*i)->contact)
			continue;

		const WheelData* data = (*i)->data;
		T_ASSERT (data != 0);

		if (!data->getDrive())
			continue;

		Vector4 direction = (*i)->direction * Vector4(1.0f, 0.0f, 1.0f, 0.0f);
		direction.normalize();

		Scalar grip = clamp((*i)->contactNormal.y(), Scalar(0.0f), Scalar(1.0f)) * Scalar((*i)->contactFudge);

		m_body->addForceAt(
			bodyTinv * (*i)->contactPosition,
			direction * engineForce * grip,
			true
		);
	}
}

void VehicleComponent::updateWheels(float dT)
{
	Transform bodyT = m_body->getTransform();

	for (RefArray< Wheel >::iterator i = m_wheels.begin(); i != m_wheels.end(); ++i)
	{
		float targetVelocity = 0.0f;

		const WheelData* data = (*i)->data;
		T_ASSERT (data != 0);

		if (!data->getDrive() || abs(m_engineThrottle) <= FUZZY_EPSILON)
			targetVelocity = (*i)->velocity * 0.95f;
		else
			targetVelocity = (m_engineThrottle * m_data->getMaxVelocity()) / data->getRadius();

		if ((*i)->contact)
		{
			float d = dot3((*i)->contactVelocity, bodyT * (*i)->direction);
			(*i)->velocity = lerp(d / data->getRadius(), targetVelocity, 0.25f);
		}
		else
			(*i)->velocity = targetVelocity;

		(*i)->angle += (*i)->velocity * dT;
	}
}

	}
}
