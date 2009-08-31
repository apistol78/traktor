#include "Animation/IK/IKPoseControllerData.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.IKPoseControllerData", IKPoseControllerData, IPoseControllerData)

IKPoseControllerData::IKPoseControllerData()
:	m_solverIterations(10)
{
}

IPoseController* IKPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Matrix44& worldTransform
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

	Ref< IKPoseController > poseController = gc_new< IKPoseController >(physicsManager, neutralPoseController, m_solverIterations);
	return poseController;
}

bool IKPoseControllerData::serialize(Serializer& s)
{
	s >> MemberRef< IPoseControllerData >(L"neutralPoseController", m_neutralPoseController);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations);
	return true;
}

	}
}
