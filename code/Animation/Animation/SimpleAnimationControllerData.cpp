/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/ITransformTimeData.h"
#include "Animation/Animation/SimpleAnimationController.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SimpleAnimationControllerData", 2, SimpleAnimationControllerData, IPoseControllerData)

SimpleAnimationControllerData::SimpleAnimationControllerData(const resource::Id< Animation >& animation)
:	m_animation(animation)
{
}

Ref< IPoseController > SimpleAnimationControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform) const
{
	resource::Proxy< Animation > animation;
	if (!resourceManager->bind(m_animation, animation))
		return nullptr;

	Ref< ITransformTime > transformTime;
	if (m_transformTime)
	{
		if ((transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	return new SimpleAnimationController(animation, transformTime);
}

void SimpleAnimationControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", m_animation);

	if (s.getVersion< SimpleAnimationControllerData >() < 1)
	{
		bool linearInterpolation;
		s >> Member< bool >(L"linearInterpolation", linearInterpolation);
	}

	if (s.getVersion< SimpleAnimationControllerData >() >= 2)
		s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
