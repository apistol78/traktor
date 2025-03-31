/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateData.h"

#include "Animation/Animation/RtState.h"
#include "Animation/IPoseControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateData", 0, RtStateData, ISerializable)

Ref< RtState > RtStateData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< RtState > instance = new RtState();
	if (m_animation)
	{
		if (resourceManager->bind(m_animation, instance->m_animation))
			return instance;
	}
	else if (m_poseController)
	{
		instance->m_poseController = m_poseController->createInstance(resourceManager, nullptr, nullptr, Transform::identity());
		if (instance->m_poseController)
			return instance;
	}
	return nullptr;
}

void RtStateData::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", m_animation);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
}

}
