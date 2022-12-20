/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/IKPoseControllerData.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.IKPoseControllerData", 0, IKPoseControllerData, IPoseControllerData)

Ref< IPoseController > IKPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
) const
{
	Ref< IPoseController > neutralPoseController;
	if (m_neutralPoseController)
	{
		neutralPoseController = m_neutralPoseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			worldTransform
		);
		if (!neutralPoseController)
			return nullptr;
	}
	return new IKPoseController(physicsManager, neutralPoseController, m_solverIterations);
}

void IKPoseControllerData::serialize(ISerializer& s)
{
	s >> MemberRef< const IPoseControllerData >(L"neutralPoseController", m_neutralPoseController);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations);
}

}
