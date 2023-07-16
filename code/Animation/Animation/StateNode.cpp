/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateContext.h"
#include "Animation/Animation/StateNode.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/IResourceManager.h"
#include "Resource/MemberIdProxy.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNode", 2, StateNode, ISerializable)

StateNode::StateNode(const std::wstring& name, const resource::IdProxy< Animation >& animation)
:	m_name(name)
,	m_position(0, 0)
,	m_animation(animation)
{
}

const std::wstring& StateNode::getName() const
{
	return m_name;
}

void StateNode::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& StateNode::getPosition() const
{
	return m_position;
}

bool StateNode::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_animation);
}

bool StateNode::prepareContext(StateContext& outContext) const
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

void StateNode::evaluate(
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

void StateNode::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< StateNode >() >= 2);

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberStlPair< int, int >(L"position", m_position);
	s >> resource::MemberIdProxy< Animation >(L"animation", m_animation);
}

}
