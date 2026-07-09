/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/AnimationGraphPoseControllerData.h"

#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/ITransformTimeData.h"
#include "Animation/Animation/RtStateGraph.h"
#include "Animation/Animation/RtStateGraphData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationGraphPoseControllerData", 0, AnimationGraphPoseControllerData, IPoseControllerData)

AnimationGraphPoseControllerData::AnimationGraphPoseControllerData(const resource::Id< RtStateGraphData >& stateGraph)
	: m_stateGraph(stateGraph)
{
}

Ref< IPoseController > AnimationGraphPoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform) const
{
	// Load the (shared, immutable) compiled state graph data through the resource manager.
	resource::Proxy< RtStateGraphData > stateGraphData;
	if (!resourceManager->bind(m_stateGraph, stateGraphData))
		return nullptr;

	// Build a per-instance runtime state graph; passing physics/skeleton/transform lets
	// physics-driven states (e.g. rag doll) create their pose controllers properly.
	Ref< RtStateGraph > stateGraph = stateGraphData->createInstance(resourceManager, physicsManager, skeleton, worldTransform);
	if (!stateGraph)
		return nullptr;

	Ref< ITransformTime > transformTime;
	if (m_transformTime)
	{
		if ((transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	return new AnimationGraphPoseController(resource::Proxy< RtStateGraph >(stateGraph), transformTime);
}

void AnimationGraphPoseControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< RtStateGraphData >(L"stateGraph", m_stateGraph);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
