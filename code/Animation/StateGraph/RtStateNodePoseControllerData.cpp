/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodePoseControllerData.h"

#include "Animation/IPoseControllerData.h"
#include "Animation/ITransformTimeData.h"
#include "Animation/StateGraph/RtStateNodePoseController.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateNodePoseControllerData", 0, RtStateNodePoseControllerData, RtStateNodeData)

Ref< RtStateNode > RtStateNodePoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	bool editor) const
{
	if (!m_poseController)
		return nullptr;

	Ref< RtStateNodePoseController > instance = new RtStateNodePoseController();

	if (m_transformTime)
	{
		if ((instance->m_transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	instance->m_poseController = m_poseController->createInstance(resourceManager, physicsManager, skeleton, worldTransform, editor);
	if (!instance->m_poseController)
		return nullptr;

	return instance;
}

void RtStateNodePoseControllerData::serialize(ISerializer& s)
{
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
