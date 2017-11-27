/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

Ref< IPoseController > StatePoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform)
{
	// Load state graph through resource manager.
	resource::Proxy< StateGraph > stateGraph;
	if (!resourceManager->bind(m_stateGraph, stateGraph))
		return 0;

	Ref< StatePoseController > poseController = new StatePoseController(stateGraph);

	// Randomize time offset; pre-evaluate controller until offset reached.
	float timeOffset = m_randomTimeOffset.random(s_random);
	for (float time = 0.0f; time < timeOffset; time += c_timeOffsetDeltaTime)
	{
		AlignedVector< Transform > dummyPose;
		bool dummyUpdate;

		poseController->evaluate(
			c_timeOffsetDeltaTime,
			worldTransform,
			skeleton,
			dummyPose,
			dummyPose,
			dummyUpdate
		);
	}

	return poseController;
}

void StatePoseControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< StateGraph >(L"stateGraph", m_stateGraph);
	if (s.getVersion() >= 1)
		s >> MemberComposite< Range< float > >(L"randomTimeOffset", m_randomTimeOffset);
}

	}
}
