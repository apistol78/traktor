/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/StatePoseControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

const float c_timeOffsetDeltaTime = 1.0f / 30.0f;
Random s_random;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePoseControllerData", 1, StatePoseControllerData, IPoseControllerData)

StatePoseControllerData::StatePoseControllerData()
:	m_randomTimeOffset(0.0f, 0.0f)
{
}

Ref< IPoseController > StatePoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform) const
{
	// Load state graph through resource manager.
	resource::Proxy< StateGraph > stateGraph;
	if (!resourceManager->bind(m_stateGraph, stateGraph))
		return nullptr;

	Ref< StatePoseController > poseController = new StatePoseController(stateGraph);

	// Randomize time offset; pre-evaluate controller until offset reached.
	float timeOffset = m_randomTimeOffset.random(s_random);
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

void StatePoseControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< StatePoseControllerData >() >= 1);
	s >> resource::Member< StateGraph >(L"stateGraph", m_stateGraph);
	s >> MemberComposite< Range< float > >(L"randomTimeOffset", m_randomTimeOffset);
}

	}
}
