/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/Stack/StackLayer.h"

#include "Animation/Animation.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/ITransformTimeData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StackLayer", 0, StackLayer, ISerializable)

StackLayer::StackLayer(const std::wstring& name, const resource::Id< Animation >& animation)
	: m_name(name)
	, m_animation(animation)
{
}

void StackLayer::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> resource::Member< Animation >(L"animation", m_animation);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", m_transformTime);
}

}
