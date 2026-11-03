/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Stack/RtStack.h"

#include "Animation/Animation.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStack", RtStack, Object)

RtStack::RtStack(const AlignedVector< Layer >& layers)
	: m_layers(layers)
{
}

void RtStack::destroy()
{
	for (auto& layer : m_layers)
		safeDestroy(layer.poseController);
	m_layers.clear();
}

void RtStack::setOwner(world::Entity* owner)
{
	for (auto& layer : m_layers)
		if (layer.poseController)
			layer.poseController->setOwner(owner);
}

void RtStack::setTransform(const Transform& transform)
{
	for (auto& layer : m_layers)
		if (layer.poseController)
			layer.poseController->setTransform(transform);
}

void RtStack::setActive(render::Handle name, bool active)
{
	for (auto& layer : m_layers)
	{
		if (layer.name == name)
		{
			layer.active = active;
			break;
		}
	}
}

bool RtStack::isActive(render::Handle name) const
{
	for (auto& layer : m_layers)
		if (layer.name == name)
			return layer.active;
	return false;
}

void RtStack::activateOnly(render::Handle name)
{
	for (auto& layer : m_layers)
		layer.active = (layer.name == name);
}

bool RtStack::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms)
{
	if (m_lastTime < 0.0f)
		m_lastTime = time;

	const float rate = 1.0f;
	const float dw = deltaTime * rate;

	// Update layer weights.
	for (auto& layer : m_layers)
	{
		layer.weight += layer.active ? dw : -dw;
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

	if (layerTop && layerBottom)
	{
		const float wa = layerTop->weight;
		const float wb = layerBottom->weight;
		const float t = (std::abs(wa + wb) > FUZZY_EPSILON) ? wb / (wa + wb) : 0.5f;
		const Scalar blend = Scalar(t);

		Pose poseTop;
		Vector4 rootMotionTop;
		layerTop->evaluate(m_lastTime, time, deltaTime, worldTransform, skeleton, jointTransforms, poseTop, rootMotionTop);

		Pose poseBottom;
		Vector4 rootMotionBottom;
		layerBottom->evaluate(m_lastTime, time, deltaTime, worldTransform, skeleton, jointTransforms, poseBottom, rootMotionBottom);

		Pose poseBlended;
		blendPoses(
			&poseTop,
			&poseBottom,
			blend,
			&poseBlended);

		calculatePoseTransforms(
			skeleton,
			&poseBlended,
			outPoseTransforms);

		m_rootMotion += rootMotionTop * (1.0_simd - blend) + rootMotionBottom * blend;
	}
	else if (layerTop)
	{
		Pose poseTop;
		Vector4 rootMotionTop;
		layerTop->evaluate(m_lastTime, time, deltaTime, worldTransform, skeleton, jointTransforms, poseTop, rootMotionTop);

		calculatePoseTransforms(
			skeleton,
			&poseTop,
			outPoseTransforms);

		m_rootMotion += rootMotionTop;
	}
	else
		return false;

	m_lastTime = time;
	return true;
}

Vector4 RtStack::consumeRootMotion()
{
	const Vector4 rootMotion = m_rootMotion;
	m_rootMotion = Vector4::zero();
	return rootMotion;
}

IPoseController* RtStack::getActivePoseController() const
{
	return nullptr;
}

void RtStack::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
	for (auto& layer : m_layers)
		if (layer.poseController)
			layer.poseController->getPoseControllersOf(type, outControllers);
}

void RtStack::Layer::evaluate(
	float lastTime,
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4& outRootMotion) const
{
	outRootMotion = Vector4::zero();

	if (animation)
	{
		const uint32_t poseCount = animation->getKeyPoseCount();
		if (poseCount < 1)
			return;

		const float duration = animation->getKeyPose(poseCount - 1).at;

		lastTime = std::fmod(lastTime, duration);
		time = std::fmod(time, duration);

		animation->getPose(time, outPose);

		const Vector4 rm0 = animation->getRootMotion(lastTime);
		const Vector4 rm1 = animation->getRootMotion(time);
		const Vector4 travel = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -rm0.w()) * (rm1 - rm0).xyz0();
		outRootMotion = Vector4(travel.x(), 0.0f, travel.z(), rm1.w() - rm0.w());
	}
	else if (poseController)
	{
		AlignedVector< Transform > poseTransforms;
		poseController->evaluate(
			time,
			deltaTime,
			worldTransform,
			skeleton,
			jointTransforms,
			poseTransforms);

		// Convert absolute transforms into a Pose instance.
		for (int32_t i = 0; i < (int32_t)skeleton->getJointCount(); ++i)
		{
			const Joint* joint = skeleton->getJoint(i);

			Transform deltaTransform = poseTransforms[i];
			if (joint->getParent() >= 0)
				deltaTransform = poseTransforms[joint->getParent()].inverse() * poseTransforms[i];

			outPose.setJointTransform(i, deltaTransform);
		}
	}
}

}
