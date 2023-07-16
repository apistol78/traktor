/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationGraph.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/AnimationGraphPoseControllerData.h"
#include "Animation/Animation/ITransformTimeData.h"
#include "Animation/Animation/StateNode.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{
	namespace
	{

const float c_timeOffsetDeltaTime = 1.0f / 30.0f;
Random s_random;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationGraphPoseControllerData", 3, AnimationGraphPoseControllerData, IPoseControllerData)

AnimationGraphPoseControllerData::AnimationGraphPoseControllerData()
:	m_randomTimeOffset(0.0f, 0.0f)
{
}

Ref< IPoseController > AnimationGraphPoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform) const
{
	// Load state graph through resource manager.
	resource::Proxy< AnimationGraph > stateGraph;
	if (!resourceManager->bind(m_animationGraph, stateGraph))
		return nullptr;

	Ref< ITransformTime > transformTime;
	if (m_transformTime)
	{
		if ((transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	Ref< AnimationGraphPoseController > poseController = new AnimationGraphPoseController(stateGraph, transformTime);

	// Randomize time offset; pre-evaluate controller until offset reached.
	const float timeOffset = m_randomTimeOffset.random(s_random);
	for (float time = 0.0f; time < timeOffset; time += c_timeOffsetDeltaTime)
	{
		AlignedVector< Transform > dummyPose;
		poseController->evaluate(
			0.0f,
			c_timeOffsetDeltaTime,
			worldTransform,
			skeleton,
			dummyPose,
			dummyPose
		);
	}

	return poseController;
}

void AnimationGraphPoseControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< AnimationGraphPoseControllerData >() >= 1);
	
	if (s.getVersion< AnimationGraphPoseControllerData >() >= 3)
		s >> resource::Member< AnimationGraph >(L"animationGraph", m_animationGraph);
	else
		s >> resource::Member< AnimationGraph >(L"stateGraph", m_animationGraph);

	s >> MemberComposite< Range< float > >(L"randomTimeOffset", m_randomTimeOffset);
	
	if (s.getVersion< AnimationGraphPoseControllerData >() >= 2)
		s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
