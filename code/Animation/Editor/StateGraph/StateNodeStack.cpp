/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateGraph/StateNodeStack.h"

#include "Animation/Animation.h"
#include "Animation/ITransformTimeData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeStack", 1, StateNodeStack, StateNode)

void StateNodeStack::serialize(ISerializer& s)
{
	StateNode::serialize(s);

	if (s.getVersion< StateNodeStack >() >= 1)
		s >> MemberComposite< Default >(L"default", m_default);

	s >> MemberAlignedVector< Layer, MemberComposite< Layer > >(L"layers", m_layers);
}

void StateNodeStack::Default::serialize(ISerializer& s)
{
	s >> resource::Member< Animation >(L"animation", animation);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", transformTime);
}

void StateNodeStack::Layer::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> resource::Member< Animation >(L"animation", animation);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", transformTime);
}

}
