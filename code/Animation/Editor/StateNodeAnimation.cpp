/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateNodeAnimation.h"

#include "Animation/Animation/Animation.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeAnimation", 0, StateNodeAnimation, StateNode)

StateNodeAnimation::StateNodeAnimation(const std::wstring& name, const resource::Id< Animation >& animation)
	: StateNode(name)
	, m_animation(animation)
{
}

void StateNodeAnimation::serialize(ISerializer& s)
{
	StateNode::serialize(s);
	s >> resource::Member< Animation >(L"animation", m_animation);
}

}
