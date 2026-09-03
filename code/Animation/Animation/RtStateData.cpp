/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateData.h"

#include "Animation/IPoseControllerData.h"
#include "Animation/Animation/ITransformTimeData.h"
#include "Animation/Animation/RtState.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateData", 1, RtStateData, ISerializable)

Ref< RtState > RtStateData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	bool editor) const
{
	Ref< RtState > instance = new RtState();

	if (m_transformTime)
	{
		if ((instance->m_transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	if (m_animation)
	{
		if (resourceManager->bind(m_animation, instance->m_animation))
			return instance;
	}
	else if (m_poseController)
	{
		instance->m_poseController = m_poseController->createInstance(resourceManager, physicsManager, skeleton, worldTransform, editor);
		if (instance->m_poseController)
			return instance;
	}

	return nullptr;
}

void RtStateData::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", m_animation);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);

	if (s.getVersion< RtStateData >() >= 1)
		s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
