/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/Pose.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationFactory", AnimationFactory, resource::IResourceFactory)

const TypeInfoSet AnimationFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< StateGraph >());
	typeSet.insert(&type_of< Animation >());
	typeSet.insert(&type_of< Skeleton >());
	typeSet.insert(&type_of< Pose >());
	return typeSet;
}

const TypeInfoSet AnimationFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet(resourceType);
}

bool AnimationFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AnimationFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< Object > object = instance->getObject();
	if (StateGraph* stateGraph = dynamic_type_cast< StateGraph* >(object))
	{
		// Ensure state node resources are loaded as well.
		const RefArray< StateNode >& states = stateGraph->getStates();
		for (RefArray< StateNode >::const_iterator i = states.begin(); i != states.end(); ++i)
		{
			if (!(*i)->bind(resourceManager))
				return 0;
		}
	}
	return object;
}

	}
}
