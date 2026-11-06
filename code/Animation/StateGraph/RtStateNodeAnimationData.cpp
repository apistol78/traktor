/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodeAnimationData.h"

#include "Animation/Animation.h"
#include "Animation/ITransformTimeData.h"
#include "Animation/StateGraph/RtStateNodeAnimation.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateNodeAnimationData", 0, RtStateNodeAnimationData, RtStateNodeData)

Ref< RtStateNode > RtStateNodeAnimationData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	bool editor) const
{
	Ref< RtStateNodeAnimation > instance = new RtStateNodeAnimation();

	if (!editor && m_transformTime)
	{
		if ((instance->m_transformTime = m_transformTime->createInstance()) == nullptr)
			return nullptr;
	}

	if (!resourceManager->bind(m_animation, instance->m_animation))
		return nullptr;

	return instance;
}

void RtStateNodeAnimationData::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", m_animation);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
