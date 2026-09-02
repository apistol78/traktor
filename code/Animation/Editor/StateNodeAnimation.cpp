/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateNodeAnimation.h"

#include "Animation/Animation/Animation.h"
#include "Animation/Animation/ITransformTimeData.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeAnimation", 2, StateNodeAnimation, StateNode)

StateNodeAnimation::StateNodeAnimation(const std::wstring& name, const resource::Id< Animation >& animation)
	: StateNode(name)
{
	m_animations.push_back(animation);
}

StateNodeAnimation::StateNodeAnimation(const std::wstring& name, const AlignedVector< resource::Id< Animation > >& animations)
	: StateNode(name)
	, m_animations(animations)
{
}

void StateNodeAnimation::serialize(ISerializer& s)
{
	StateNode::serialize(s);

	if (s.getVersion< StateNodeAnimation >() >= 2)
	{
		s >> MemberAlignedVector< resource::Id< Animation >, resource::Member< Animation > >(L"animations", m_animations);
		s >> Member< float >(L"loopDuration", m_loopDuration, AttributeRange(0.0f) | AttributeUnit(UnitType::Seconds));
	}
	else
	{
		resource::Id< Animation > animation;
		s >> resource::Member< Animation >(L"animation", animation);
		if (animation)
			m_animations.push_back(animation);
	}

	if (s.getVersion< StateNodeAnimation >() >= 1)
		s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
