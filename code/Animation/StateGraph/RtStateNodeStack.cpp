/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodeStack.h"

#include "Animation/Animation.h"
#include "Animation/ITransformTime.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/StateGraph/RtStateContext.h"
#include "Core/Misc/SafeDestroy.h"

#include <limits>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateNodeStack", RtStateNodeStack, RtStateNode)

RtStateNodeStack::RtStateNodeStack(const AlignedVector< RtStateNodeStack::Layer >& layers)
	: m_layers(layers)
{
}

void RtStateNodeStack::destroy()
{
	m_layers.clear();
}

void RtStateNodeStack::reset(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms)
{
	for (auto& layer : m_layers)
	{
		if (!layer.animation)
			continue;

		const uint32_t poseCount = layer.animation->getKeyPoseCount();
		if (poseCount < 1)
			continue;

		layer.duration = layer.animation->getKeyPose(poseCount - 1).at;
		layer.time = 0.0f;
		layer.realTime = 0.0f;
		layer.lastTime = 0.0f;
		layer.weight = 0.0f;
	}
}

float RtStateNodeStack::evaluate(
	const RtStateContext& context,
	float /*time*/,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4* outRootMotion)
{
	const float rate = 1.0f;
	const float dw = deltaTime * rate;

	// Update layers.
	for (auto& layer : m_layers)
	{
		const bool active = context.getValues()[layer.condition];

		float T = layer.realTime;
		float dT = deltaTime;

		if (layer.transformTime)
			layer.transformTime->calculateTime(layer.animation, worldTransform, T, dT);

		layer.lastTime = layer.time;
		layer.time = T;
		layer.realTime += deltaTime;

		layer.weight += active ? dw : -dw;
		layer.weight = clamp(layer.weight, 0.0f, 1.0f);
	}

	// Find two top most weighted layers.
	const Layer* layerTop = nullptr;
	const Layer* layerBottom = nullptr;

	float layerTopWeight = 0.0f;
	float layerBottomWeight = 0.0f;

	for (auto& layer : m_layers)
	{
		const float lw = layer.weight - FUZZY_EPSILON;
		if (lw > layerTopWeight)
		{
			if (layerTopWeight > layerBottomWeight)
			{
				layerBottom = layerTop;
				layerBottomWeight = layerTopWeight;
			}

			layerTop = &layer;
			layerTopWeight = layer.weight;
		}
		else if (lw > layerBottomWeight)
		{
			layerBottom = &layer;
			layerBottomWeight = layer.weight;
		}
	}

	if (outRootMotion)
		*outRootMotion = Vector4::zero();

	if (layerTop && layerBottom)
	{
		const float wa = layerTop->weight;
		const float wb = layerBottom->weight;
		const float t = (std::abs(wa + wb) > FUZZY_EPSILON) ? wb / (wa + wb) : 0.5f;
		const Scalar blend = Scalar(t);

		Pose poseTop;
		Vector4 rootMotionTop;
		layerTop->evaluate(worldTransform, skeleton, jointTransforms, poseTop, rootMotionTop);

		Pose poseBottom;
		Vector4 rootMotionBottom;
		layerBottom->evaluate(worldTransform, skeleton, jointTransforms, poseBottom, rootMotionBottom);

		blendPoses(
			&poseTop,
			&poseBottom,
			blend,
			&outPose);

		if (outRootMotion)
			*outRootMotion = rootMotionTop * (1.0_simd - blend) + rootMotionBottom * blend;
	}
	else if (layerTop)
	{
		Vector4 rootMotionTop;
		layerTop->evaluate(worldTransform, skeleton, jointTransforms, outPose, rootMotionTop);
		if (outRootMotion)
			*outRootMotion = rootMotionTop;
	}

	// Layers loop by themselves; the stack never ends on its own.
	return std::numeric_limits< float >::max();
}

IPoseController* RtStateNodeStack::getActivePoseController() const
{
	return nullptr;
}

void RtStateNodeStack::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
}

void RtStateNodeStack::Layer::evaluate(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4& outRootMotion) const
{
	const float loopedLastTime = std::fmod(lastTime, duration);
	const float loopedTime = std::fmod(time, duration);

	animation->getPose(loopedTime, outPose);

	outRootMotion = Vector4::zero();
	if (loopedTime <= loopedLastTime)
		return;

	const Vector4 rm0 = animation->getRootMotion(loopedLastTime);
	const Vector4 rm1 = animation->getRootMotion(loopedTime);
	const Vector4 travel = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -rm0.w()) * (rm1 - rm0).xyz0();
	outRootMotion = Vector4(travel.x(), 0.0f, travel.z(), rm1.w() - rm0.w());
}

}
