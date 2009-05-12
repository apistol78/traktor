#include "Animation/Animation/StatePoseControllerData.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/StateGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.StatePoseControllerData", StatePoseControllerData, PoseControllerData)

PoseController* StatePoseControllerData::createInstance(physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Matrix44& worldTransform)
{
	return gc_new< StatePoseController >(cref(m_stateGraph));
}

bool StatePoseControllerData::serialize(Serializer& s)
{
	return s >> resource::Member< StateGraph >(L"stateGraph", m_stateGraph);
}

	}
}
