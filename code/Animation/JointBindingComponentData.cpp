/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/JointBindingComponent.h"
#include "Animation/JointBindingComponentData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.JointBindingComponentData", 0, JointBindingComponentData, world::IEntityComponentData)

Ref< JointBindingComponent > JointBindingComponentData::createComponent(const world::IEntityBuilder* entityBuilder) const
{
	AlignedVector< JointBindingComponent::Binding > bindings;
	for (size_t i = 0; i < m_bindings.size(); ++i)
	{
		Ref< world::Entity > entity = entityBuilder->create(m_bindings[i].entityData);
		if (entity)
		{
			JointBindingComponent::Binding binding;
			binding.jointHandle = render::getParameterHandle(m_bindings[i].jointName);
			binding.entity = entity;
			bindings.push_back(binding);
		}
	}

	return new JointBindingComponent(bindings);
}

int32_t JointBindingComponentData::getOrdinal() const
{
	return 100;
}

void JointBindingComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void JointBindingComponentData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Binding, MemberComposite< Binding > >(L"bindings", m_bindings);
}

void JointBindingComponentData::Binding::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> MemberRef< const world::EntityData >(L"entityData", entityData);
}

}
