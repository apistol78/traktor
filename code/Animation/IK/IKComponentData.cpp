/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/IKComponent.h"
#include "Animation/IK/IKComponentData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.IKComponentData", 0, IKComponentData, world::IEntityComponentData)

Ref< IKComponent > IKComponentData::createComponent() const
{
	Ref< IKComponent > component = new IKComponent();
	for (const auto& target : m_targets)
		component->setTarget(render::getParameterHandle(target.jointName), target.position);
	return component;
}

int32_t IKComponentData::getOrdinal() const
{
	return -50;
}

void IKComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void IKComponentData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Target, MemberComposite< Target > >(L"targets", m_targets);
}

void IKComponentData::Target::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> Member< Vector4 >(L"position", position);
}

}
