/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Skeleton.h"
#include "Animation/Animation/RetargetPoseController.h"
#include "Animation/Animation/RetargetPoseControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RetargetPoseControllerData", 2, RetargetPoseControllerData, IPoseControllerData)

RetargetPoseControllerData::RetargetPoseControllerData(const resource::Id< Skeleton >& animationSkeleton)
:	m_animationSkeleton(animationSkeleton)
{
}

Ref< IPoseController > RetargetPoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform) const
{
	if (!m_poseController)
		return nullptr;

	resource::Proxy< Skeleton > animationSkeleton;
	if (!resourceManager->bind(m_animationSkeleton, animationSkeleton))
		return nullptr;

	Ref< IPoseController > poseController = m_poseController->createInstance(resourceManager, physicsManager, animationSkeleton, worldTransform);
	if (!poseController)
		return nullptr;

	return new RetargetPoseController(animationSkeleton, poseController);
}

void RetargetPoseControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< Skeleton >(L"animationSkeleton", m_animationSkeleton);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
}

}
