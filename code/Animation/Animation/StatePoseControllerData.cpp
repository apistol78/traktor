#include "Animation/Animation/StatePoseControllerData.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/State.h"
#include "Animation/Animation/Animation.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.StatePoseControllerData", StatePoseControllerData, IPoseControllerData)

Ref< IPoseController > StatePoseControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform)
{
	if (!resourceManager->bind(m_stateGraph))
		return 0;

	const RefArray< State >& states = m_stateGraph->getStates();
	for (RefArray< State >::const_iterator i = states.begin(); i != states.end(); ++i)
	{
		if (!resourceManager->bind((*i)->getAnimation()))
			return 0;
	}

	return gc_new< StatePoseController >(cref(m_stateGraph));
}

bool StatePoseControllerData::serialize(Serializer& s)
{
	return s >> resource::Member< StateGraph >(L"stateGraph", m_stateGraph);
}

	}
}
