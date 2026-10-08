/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/LookAtComponent.h"

#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Math/Const.h"
#include "Core/Math/Quaternion.h"
#include "World/Entity.h"

#include <cmath>

namespace traktor::animation
{
namespace
{

// The gaze blends in and out slower than it settles, so enabling doesn't snap the head.
const float c_weightTimeScale = 2.0f;

const Vector4 c_forward(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 c_up(0.0f, 1.0f, 0.0f, 0.0f);

float smoothTowards(float current, float target, float time, float deltaTime)
{
	if (time <= FUZZY_EPSILON)
		return target;
	const float k = 1.0f - std::exp(-deltaTime / time);
	return current + (target - current) * k;
}

float wrapAngle(float angle)
{
	while (angle > PI)
		angle -= TWO_PI;
	while (angle < -PI)
		angle += TWO_PI;
	return angle;
}

float yawOf(const Vector4& v)
{
	return std::atan2((float)v.x(), (float)v.z());
}

float pitchOf(const Vector4& v)
{
	const float h = std::sqrt((float)(v.x() * v.x() + v.z() * v.z()));
	return std::atan2((float)v.y(), h);
}

Vector4 directionOf(float yaw, float pitch)
{
	return Vector4(
		std::sin(yaw) * std::cos(pitch),
		std::sin(pitch),
		std::cos(yaw) * std::cos(pitch),
		0.0f);
}

Vector4 gazeOffset(float yaw, float pitch, const Vector4& offset)
{
	const float sy = std::sin(yaw), cy = std::cos(yaw);
	const float sp = std::sin(pitch), cp = std::cos(pitch);
	const Vector4 right(cy, 0.0f, -sy, 0.0f);
	const Vector4 up(-sy * sp, cp, -cy * sp, 0.0f);
	const Vector4 forward(sy * cp, sp, cy * cp, 0.0f);
	return right * offset.x() + up * offset.y() + forward * offset.z();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LookAtComponent", LookAtComponent, world::IEntityComponent)

LookAtComponent::LookAtComponent(const Settings& settings)
	: m_settings(settings)
{
}

void LookAtComponent::destroy()
{
}

void LookAtComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void LookAtComponent::setTransform(const Transform& transform)
{
}

Aabb3 LookAtComponent::getBoundingBox() const
{
	return Aabb3();
}

void LookAtComponent::update(const world::UpdateParams& update)
{
	if (m_settings.joint == 0)
		return;

	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return;

	const auto& skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return;

	// Beyond the cull distance the head is left to the animation; start over
	// from the animated head when coming back.
	auto animatedMeshComponent = m_owner->getComponent< AnimatedMeshComponent >();
	if (animatedMeshComponent && animatedMeshComponent->getLastDistance() >= m_settings.cullDistance)
	{
		m_weight = 0.0f;
		return;
	}

	// The pose may still be evaluating in a job; wait for it before reading.
	skeletonComponent->synchronize();

	// Pose not evaluated since we wrote it, as with a reduced evaluation rate; it
	// still carries our gaze, which mustn't be applied twice.
	if (skeletonComponent->getRevision() == m_revision)
		return;

	uint32_t index;
	if (!skeleton->findJoint(m_settings.joint, index))
		return;

	const int32_t parent = skeleton->getJoint(index)->getParent();
	if (parent < 0)
		return;

	const AlignedVector< Transform >& jointTransforms = skeletonComponent->getJointTransforms();
	const AlignedVector< Transform >& pose = skeletonComponent->getPoseTransforms();
	if (index >= pose.size() || index >= jointTransforms.size())
		return;

	const float deltaTime = (float)update.deltaTime;
	const Transform ownerTransform = m_owner->getTransform();
	const Quaternion ownerRotation = ownerTransform.rotation();
	const Quaternion ownerRotationInv = ownerRotation.inverse();

	// Forward of the head and the chest, in object space; whatever pointed along +Z in the bind pose.
	const Vector4 head = pose[index].rotation() * (jointTransforms[index].rotation().inverse() * c_forward);
	const Vector4 chest = pose[parent].rotation() * (jointTransforms[parent].rotation().inverse() * c_forward);
	const float headYaw = yawOf(head);
	const float chestYaw = yawOf(chest);

	const auto limitedGaze = [&](float worldYaw, float worldPitch, float& outYaw, float& outPitch) {
		const Vector4 direction = ownerRotationInv * directionOf(worldYaw, worldPitch);
		outYaw = chestYaw + clamp(wrapAngle(yawOf(direction) - chestYaw), -m_settings.maxYaw, m_settings.maxYaw);
		outPitch = clamp(pitchOf(direction), -m_settings.maxPitch, m_settings.maxPitch);
	};

	if (m_weight <= FUZZY_EPSILON)
	{
		const Vector4 headWorld = ownerRotation * head;
		m_gazeYaw = yawOf(headWorld);
		m_gazePitch = pitchOf(headWorld);
	}

	const Vector4 neckPosition = pose[index].translation().xyz1();
	const Vector4 targetObject = ownerTransform.inverse() * m_target.xyz1();

	float aimYaw, aimPitch;
	limitedGaze(m_gazeYaw, m_gazePitch, aimYaw, aimPitch);
	Vector4 toTarget = Vector4::zero();

	const Vector4 eyePosition = neckPosition + gazeOffset(aimYaw, aimPitch, m_settings.eyeOffset);
	toTarget = (targetObject - eyePosition).xyz0();
	if (toTarget.length() > FUZZY_EPSILON)
	{
		aimYaw = yawOf(toTarget);
		aimPitch = pitchOf(toTarget);
	}

	if (m_enable && toTarget.length() > FUZZY_EPSILON)
	{
		const float yaw = chestYaw + clamp(wrapAngle(aimYaw - chestYaw), -m_settings.maxYaw, m_settings.maxYaw);
		const float pitch = clamp(aimPitch, -m_settings.maxPitch, m_settings.maxPitch);
		const Vector4 targetWorld = ownerRotation * directionOf(yaw, pitch);
		m_gazeYaw = wrapAngle(m_gazeYaw + smoothTowards(0.0f, wrapAngle(yawOf(targetWorld) - m_gazeYaw), m_settings.smoothTime, deltaTime));
		m_gazePitch = smoothTowards(m_gazePitch, pitchOf(targetWorld), m_settings.smoothTime, deltaTime);
	}

	m_weight = smoothTowards(m_weight, m_enable ? 1.0f : 0.0f, m_settings.smoothTime * c_weightTimeScale, deltaTime);
	if (!m_enable && m_weight < 0.01f)
		m_weight = 0.0f;
	if (m_weight <= FUZZY_EPSILON)
		return;

	float gazeYaw, gazePitch;
	limitedGaze(m_gazeYaw, m_gazePitch, gazeYaw, gazePitch);
	const Vector4 gaze = directionOf(gazeYaw, gazePitch);

	const Quaternion turn = Quaternion::fromAxisAngle(c_up, wrapAngle(gazeYaw - headYaw));
	const Vector4 turned = turn * head;
	const Scalar turnedLength = turned.length();
	if (turnedLength < FUZZY_EPSILON)
		return;

	const Quaternion tilt = Quaternion(turned / turnedLength, gaze).normalized();
	Quaternion rotation = (tilt * turn).normalized();
	if (m_weight < 1.0f)
		rotation = slerp(Quaternion::identity(), rotation, m_weight);

	// Rotate the joint about its own position and carry its subtree along.
	const Transform neck = pose[index];
	const Transform transform(neck.translation(), rotation * neck.rotation());
	skeletonComponent->concatenatePoseTransform(m_settings.joint, neck.inverse() * transform, true);
	m_revision = skeletonComponent->getRevision();
}

}
