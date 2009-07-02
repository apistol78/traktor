#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/Pose.h"
#include "Database/Database.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationFactory", AnimationFactory, resource::IResourceFactory)

AnimationFactory::AnimationFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet AnimationFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< StateGraph >());
	typeSet.insert(&type_of< Animation >());
	typeSet.insert(&type_of< Skeleton >());
	typeSet.insert(&type_of< Pose >());
	return typeSet;
}

bool AnimationFactory::isCacheable() const
{
	return true;
}

Object* AnimationFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	return m_db->getObjectReadOnly(guid);
}

	}
}
