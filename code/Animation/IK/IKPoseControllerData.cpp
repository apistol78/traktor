/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/IK/IKPoseControllerData.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.IKPoseControllerData", 0, IKPoseControllerData, IPoseControllerData)

IKPoseControllerData::IKPoseControllerData()
:	m_solverIterations(10)
{
}

Ref< IPoseController > IKPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
)
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
			return 0;
	}

	Ref< IKPoseController > poseController = new IKPoseController(physicsManager, neutralPoseController, m_solverIterations);
	return poseController;
}

void IKPoseControllerData::serialize(ISerializer& s)
{
	s >> MemberRef< IPoseControllerData >(L"neutralPoseController", m_neutralPoseController);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations);
}

	}
}
