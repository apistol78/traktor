#include "Animation/Animation/Animation.h"
#include "Animation/Animation/State.h"
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
	if (!resourceManager->bind(m_stateGraph))
		return 0;

	if (!m_stateGraph.validate())
		return 0;

	const RefArray< State >& states = m_stateGraph->getStates();
	for (RefArray< State >::const_iterator i = states.begin(); i != states.end(); ++i)
	{
		if (!resourceManager->bind((*i)->getAnimation()))
			return 0;
	}

	Ref< StatePoseController > poseController = new StatePoseController(m_stateGraph);

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

bool StatePoseControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< StateGraph >(L"stateGraph", m_stateGraph);

	if (s.getVersion() >= 1)
		s >> MemberComposite< Range< float > >(L"randomTimeOffset", m_randomTimeOffset);

	return true;
}

	}
}
