/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/SimpleAnimationController.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SimpleAnimationControllerData", 0, SimpleAnimationControllerData, IPoseControllerData)

SimpleAnimationControllerData::SimpleAnimationControllerData()
:	m_linearInterpolation(false)
{
}

Ref< IPoseController > SimpleAnimationControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform)
{
	resource::Proxy< Animation > animation;
	if (resourceManager->bind(m_animation, animation))
		return new SimpleAnimationController(animation, m_linearInterpolation);
	else
		return 0;
}

void SimpleAnimationControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", m_animation);
	s >> Member< bool >(L"linearInterpolation", m_linearInterpolation);
}

	}
}
