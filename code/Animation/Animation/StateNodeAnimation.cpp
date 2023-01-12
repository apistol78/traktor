/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateContext.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Resource/MemberIdProxy.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeAnimation", 2, StateNodeAnimation, StateNode)

StateNodeAnimation::StateNodeAnimation(const std::wstring& name, const resource::IdProxy< Animation >& animation)
:	StateNode(name)
,	m_animation(animation)
{
}

bool StateNodeAnimation::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_animation);
}

bool StateNodeAnimation::prepareContext(StateContext& outContext) const
{
	if (!m_animation)
		return false;

	const uint32_t poseCount = m_animation->getKeyPoseCount();
	if (poseCount < 1)
		return false;

	const float duration = m_animation->getKeyPose(poseCount - 1).at;

	outContext.setTime(0.0f);
	outContext.setDuration(duration);

	return true;
}

void StateNodeAnimation::evaluate(
	StateContext& context,
	Pose& outPose
) const
{
	const float time = context.getTime();
	int32_t indexHint = context.getIndexHint();

	if (m_animation)
		m_animation->getPose(time, indexHint, outPose);

	context.setIndexHint(indexHint);
}

void StateNodeAnimation::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);

	StateNode::serialize(s);

	s >> resource::MemberIdProxy< Animation >(L"animation", m_animation);

	if (s.getVersion< StateNodeAnimation >() < 2)
	{
		bool linearInterpolation;
		s >> Member< bool >(L"linearInterpolation", linearInterpolation);
	}
}

}
