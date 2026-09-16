/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/FootPlacementComponent.h"

#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Math/Const.h"
#include "Physics/PhysicsManager.h"
#include "World/Entity.h"

namespace traktor::animation
{
namespace
{

// Fraction of the full leg length a foot may reach; keeps the knee from locking
// straight, which reads as a snap and leaves the solve without a bend direction.
const float c_maxExtension = 0.995f;

// Fraction of the cull distance over which the correction fades out, so an
// entity crossing the boundary doesn't pop.
const float c_cullFade = 0.2f;

// A correction that lifts a foot out of the ground settles this much faster
// than one lowering a floating foot; penetration is the more visible error.
const float c_liftRateScale = 0.5f;

// The pelvis is heavier than a foot and settles slower.
const float c_pelvisTimeScale = 2.0f;

const Vector4 c_worldUp(0.0f, 1.0f, 0.0f, 0.0f);

/*! Exponential approach; framerate independent. */
float smoothTowards(float current, float target, float time, float deltaTime)
{
	if (time <= FUZZY_EPSILON)
		return target;
	const float k = 1.0f - std::exp(-deltaTime / time);
	return current + (target - current) * k;
}

Vector4 smoothTowards(const Vector4& current, const Vector4& target, float time, float deltaTime)
{
	if (time <= FUZZY_EPSILON)
		return target;
	const float k = 1.0f - std::exp(-deltaTime / time);
	return lerp(current, target, Scalar(k));
}

/*! Replace the transform of a joint and carry its subtree rigidly along.
 *
 * Pose transforms are in object space, so every descendant is re-based by
 * the delta of the moved joint; unlike SkeletonComponent::setPoseTransform
 * with inclusive set this preserves the animated pose of the descendants.
 */
void moveSubtree(const Skeleton* skeleton, AlignedVector< Transform >& pose, uint32_t index, const Transform& transform)
{
	const Transform delta = transform * pose[index].inverse();
	pose[index] = transform;
	skeleton->findAllChildren(index, [&](uint32_t child) {
		pose[child] = delta * pose[child];
	});
}

/*! Rotate a joint about its own position so a direction from it aligns with another. */
void rotateSubtree(const Skeleton* skeleton, AlignedVector< Transform >& pose, uint32_t index, const Vector4& from, const Vector4& to)
{
	const Scalar fromLength = from.length();
	const Scalar toLength = to.length();
	if (fromLength < FUZZY_EPSILON || toLength < FUZZY_EPSILON)
		return;

	const Quaternion rotation = Quaternion(from / fromLength, to / toLength).normalized();
	moveSubtree(skeleton, pose, index, Transform(
		pose[index].translation(),
		rotation * pose[index].rotation()
	));
}

/*! Move a foot to a target with an analytic two bone solve of hip and knee.
 *
 * The knee bends in the plane the animation already bends it in, so knees
 * keep pointing the way the animator intended without a pole target. With a
 * straight animated leg the toe direction is used instead, since knees bend
 * the way the toes point. The foot ends up with the given orientation.
 */
void solveLeg(
	const Skeleton* skeleton,
	const AlignedVector< Transform >& bind,
	AlignedVector< Transform >& pose,
	uint32_t foot,
	const Vector4& target,
	const Quaternion& footRotation
)
{
	const int32_t knee = skeleton->getJoint(foot)->getParent();
	const int32_t hip = (knee >= 0) ? skeleton->getJoint(knee)->getParent() : -1;
	if (hip < 0)
	{
		// No leg chain above the foot; snapping the foot is all that's possible.
		moveSubtree(skeleton, pose, foot, Transform(target.xyz0(), footRotation));
		return;
	}

	const Vector4 hipPosition = pose[hip].translation().xyz0();
	const Vector4 kneePosition = pose[knee].translation().xyz0();
	const Vector4 footPosition = pose[foot].translation().xyz0();

	// Bone lengths from the bind pose; the animation may have stretched them slightly.
	const Scalar thighLength = (bind[knee].translation() - bind[hip].translation()).length();
	const Scalar shinLength = (bind[foot].translation() - bind[knee].translation()).length();
	if (thighLength < FUZZY_EPSILON || shinLength < FUZZY_EPSILON)
		return;

	// Direction the knee is bent in, taken from the animated pose.
	Vector4 bend = Vector4::zero();
	{
		const Vector4 axis = footPosition - hipPosition;
		const Scalar axisLength = axis.length();
		if (axisLength > FUZZY_EPSILON)
		{
			const Vector4 axisN = axis / axisLength;
			const Vector4 thigh = kneePosition - hipPosition;
			bend = thigh - axisN * dot3(thigh, axisN);
		}
	}
	if (bend.length() < 0.001_simd)
	{
		// Straight leg; bend toward the toes if there are any, else hip forward.
		bend = pose[hip].rotation() * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		skeleton->findChildren(foot, [&](uint32_t toe) {
			bend = pose[toe].translation().xyz0() - footPosition;
		});
	}

	// Clamp the reach so the leg neither locks straight nor folds past its bones.
	const Vector4 toTarget = target.xyz0() - hipPosition;
	const Scalar distance = toTarget.length();
	if (distance < FUZZY_EPSILON)
		return;

	const Scalar reach = clamp(
		distance,
		abs(thighLength - shinLength) + Scalar(FUZZY_EPSILON),
		(thighLength + shinLength) * Scalar(c_maxExtension)
	);
	const Vector4 axis = toTarget / distance;
	const Vector4 footTarget = hipPosition + axis * reach;

	// Bend direction orthogonal to the new hip to foot axis.
	bend = bend - axis * dot3(bend, axis);
	const Scalar bendLength = bend.length();
	if (bendLength < FUZZY_EPSILON)
		return;
	bend /= bendLength;

	// Law of cosines; angle at the hip between the axis and the thigh.
	const float l1 = (float)thighLength;
	const float l2 = (float)shinLength;
	const float r = (float)reach;
	const float cosA = clamp((l1 * l1 + r * r - l2 * l2) / (2.0f * l1 * r), -1.0f, 1.0f);
	const float sinA = std::sqrt(std::max(1.0f - cosA * cosA, 0.0f));
	const Vector4 kneeTarget = hipPosition + (axis * Scalar(cosA) + bend * Scalar(sinA)) * thighLength;

	// Thigh; swing the hip so the knee lands on its target, carrying shin and foot.
	rotateSubtree(skeleton, pose, hip, kneePosition - hipPosition, kneeTarget - hipPosition);

	// Shin; swing the knee so the foot lands on its target.
	const Vector4 kneeNow = pose[knee].translation().xyz0();
	rotateSubtree(skeleton, pose, knee, pose[foot].translation().xyz0() - kneeNow, footTarget - kneeNow);

	// The foot gets its own orientation rather than tilting with the shin.
	moveSubtree(skeleton, pose, foot, Transform(pose[foot].translation(), footRotation));
}

/*! Rotation tilting up onto the ground normal, limited to a maximum angle
 * and scaled by weight. */
Quaternion groundTilt(const Vector4& up, const Vector4& normal, float maxTilt, float weight)
{
	const Scalar normalLength = normal.length();
	if (normalLength < FUZZY_EPSILON || weight <= FUZZY_EPSILON)
		return Quaternion::identity();

	Quaternion tilt = Quaternion(up, normal / normalLength).normalized();

	const Vector4 axisAngle = tilt.toAxisAngle();
	const float angle = (float)axisAngle.length();
	if (angle < FUZZY_EPSILON)
		return Quaternion::identity();
	if (angle > maxTilt)
		tilt = Quaternion::fromAxisAngle(axisAngle / Scalar(angle), maxTilt);

	return (weight < 1.0f) ? slerp(Quaternion::identity(), tilt, weight) : tilt;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.FootPlacementComponent", FootPlacementComponent, world::IEntityComponent)

FootPlacementComponent::FootPlacementComponent(
	physics::PhysicsManager* physicsManager,
	const AlignedVector< render::handle_t >& footJoints,
	const Settings& settings
)
	: m_physicsManager(physicsManager)
	, m_footJoints(footJoints)
	, m_settings(settings)
{
	m_footStates.resize(m_footJoints.size());
	for (auto& state : m_footStates)
		state.normal = c_worldUp;
}

void FootPlacementComponent::destroy()
{
	m_physicsManager = nullptr;
}

void FootPlacementComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void FootPlacementComponent::setTransform(const Transform& transform)
{
}

Aabb3 FootPlacementComponent::getBoundingBox() const
{
	return Aabb3();
}

void FootPlacementComponent::update(const world::UpdateParams& update)
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return;

	auto animatedMeshComponent = m_owner->getComponent< AnimatedMeshComponent >();
	if (!animatedMeshComponent)
		return;

	// Fade the correction out toward the cull distance and skip beyond it. The
	// smoothed state is reset so a returning entity starts clean.
	const float distance = animatedMeshComponent->getLastDistance();
	if (distance >= m_settings.cullDistance)
	{
		for (auto& state : m_footStates)
			state = { 0.0f, c_worldUp };
		m_pelvisOffset = 0.0f;
		return;
	}
	const float fadeDistance = std::max(m_settings.cullDistance * c_cullFade, FUZZY_EPSILON);
	const float distanceWeight = clamp((m_settings.cullDistance - distance) / fadeDistance, 0.0f, 1.0f);

	const auto& skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return;

	// The pose may still be evaluating in a job; wait for it before reading.
	skeletonComponent->synchronize();

	const uint32_t jointCount = skeleton->getJointCount();
	const AlignedVector< Transform >& jointTransforms = skeletonComponent->getJointTransforms();
	AlignedVector< Transform > pose = skeletonComponent->getPoseTransforms();
	if (jointTransforms.size() < jointCount || pose.size() < jointCount)
		return;

	const float deltaTime = (float)update.deltaTime;
	const Transform ownerTransform = m_owner->getTransform();
	const Transform ownerTransformInv = ownerTransform.inverse();
	const Vector4 upObject = (ownerTransformInv.rotation() * c_worldUp).normalized();

	// 1. Trace each foot and settle its correction along up and its ground normal.
	struct Foot
	{
		uint32_t index;
		Vector4 target;
		Quaternion rotation;
	};
	AlignedVector< Foot > feet;
	float pelvisTarget = 0.0f;

	for (uint32_t i = 0; i < (uint32_t)m_footJoints.size(); ++i)
	{
		FootState& state = m_footStates[i];

		uint32_t index;
		if (!skeleton->findJoint(m_footJoints[i], index))
			continue;

		const Transform& animatedFoot = pose[index];
		const Vector4 footObject = animatedFoot.translation().xyz1();

		// Weight from the animation; a foot the animator has lifted above the
		// character's ground plane is in a swing phase and is left alone. The
		// entity origin is assumed to be at ground level as for all characters.
		const float soleHeight = (float)dot3(footObject, upObject) - m_settings.offset;
		float weight = distanceWeight;
		if (m_settings.liftHeight > FUZZY_EPSILON)
			weight *= clamp(1.0f - soleHeight / m_settings.liftHeight, 0.0f, 1.0f);
		else if (soleHeight > 0.0f)
			weight = 0.0f;

		// Trace from rangeUp above the sole to rangeDown below it.
		float targetOffset = 0.0f;
		Vector4 targetNormal = upObject;
		bool planted = false;
		if (weight > 0.0f)
		{
			const Vector4 footWorld = ownerTransform * footObject;

			physics::QueryResult result;
			if (m_physicsManager->queryRay(
				footWorld + c_worldUp * Scalar(m_settings.rangeUp - m_settings.offset),
				-c_worldUp,
				m_settings.rangeUp + m_settings.rangeDown,
				physics::QueryFilter(m_settings.traceInclude, m_settings.traceIgnore),
				false,
				result
			))
			{
				targetOffset = ((float)result.position.y() + m_settings.offset - (float)footWorld.y()) * weight;
				targetNormal = (ownerTransformInv.rotation() * result.normal.xyz0()).normalized();
				planted = true;
			}
		}

		// Settle; lifting out of the ground settles faster than lowering onto it.
		const float offsetTime = (targetOffset > state.offset) ? m_settings.smoothTime * c_liftRateScale : m_settings.smoothTime;
		state.offset = smoothTowards(state.offset, targetOffset, offsetTime, deltaTime);
		state.normal = smoothTowards(state.normal, targetNormal, m_settings.smoothTime, deltaTime);

		if (planted)
			pelvisTarget = std::min(pelvisTarget, targetOffset);

		if (std::abs(state.offset) < FUZZY_EPSILON && weight <= FUZZY_EPSILON)
			continue;

		const Quaternion tilt = groundTilt(upObject, state.normal, m_settings.maxTilt, weight);
		feet.push_back({ index, footObject + upObject * Scalar(state.offset), tilt * animatedFoot.rotation() });
	}

	// 2. Lower the pelvis by the deepest foot so that leg can reach without
	// stretching; the other legs bend more to stay on their own targets.
	pelvisTarget = std::max(pelvisTarget, -m_settings.pelvisDropMax);
	m_pelvisOffset = smoothTowards(m_pelvisOffset, pelvisTarget, m_settings.smoothTime * c_pelvisTimeScale, deltaTime);

	if (feet.empty() && std::abs(m_pelvisOffset) < FUZZY_EPSILON)
		return;

	uint32_t hips;
	if (m_settings.hipsJoint != 0 && skeleton->findJoint(m_settings.hipsJoint, hips))
	{
		moveSubtree(skeleton, pose, hips, Transform(
			pose[hips].translation() + upObject * Scalar(m_pelvisOffset),
			pose[hips].rotation()
		));
	}

	// 3. Solve each leg onto its target.
	for (const auto& foot : feet)
		solveLeg(skeleton, jointTransforms, pose, foot.index, foot.target, foot.rotation);

	skeletonComponent->setPoseTransforms(pose);
}

}
