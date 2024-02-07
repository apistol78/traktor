/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/JointBindingComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Misc/SafeDestroy.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.JointBindingComponent", JointBindingComponent, world::IEntityComponent)

JointBindingComponent::JointBindingComponent(const AlignedVector< Binding >& bindings)
:	m_bindings(bindings)
{
}

void JointBindingComponent::destroy()
{
	for (auto& binding : m_bindings)
	{
		if (binding.entity)
			binding.entity->destroy();
	}
	m_bindings.resize(0);
}

void JointBindingComponent::setOwner(world::Entity* owner)
{
	T_ASSERT(m_owner == nullptr);
	if ((m_owner = owner) != nullptr)
	{
		m_transform = m_owner->getTransform();

		auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
		if (skeletonComponent)
		{
			for (auto& binding : m_bindings)
			{
				if (!skeletonComponent->getPoseTransform(binding.jointHandle, binding.poseTransform))
					binding.poseTransform = Transform::identity();
			}
		}
	}
}

void JointBindingComponent::setTransform(const Transform& transform)
{
	const Transform invTransform = m_transform.inverse();
	for (auto& binding : m_bindings)
	{
		const Transform currentTransform = binding.entity->getTransform();
		const Transform Tlocal = invTransform * currentTransform;
		const Transform Tworld = transform * Tlocal;
		binding.entity->setTransform(Tworld);
	}
	m_transform = transform;
}

Aabb3 JointBindingComponent::getBoundingBox() const
{
	Aabb3 worldBoundingBox;
	for (auto& binding : m_bindings)
	{
		const Aabb3 childBoundingBox = binding.entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			const Transform childTransform = binding.entity->getTransform();
			worldBoundingBox.contain(childBoundingBox.transform(childTransform));
		}
	}
	return worldBoundingBox.transform(m_transform.inverse());
}

void JointBindingComponent::update(const world::UpdateParams& update)
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (skeletonComponent)
	{
		for (auto& binding : m_bindings)
		{
			Transform T;
			if (skeletonComponent->getPoseTransform(binding.jointHandle, T))
			{
				const Transform invPoseTransform = (m_transform * binding.poseTransform).inverse();
				const Transform currentTransform = binding.entity->getTransform();
				const Transform Tlocal = invPoseTransform * currentTransform;
				const Transform Tworld = (m_transform * T) * Tlocal;
				binding.entity->setTransform(Tworld);
				binding.poseTransform = T;
			}
		}
	}
}

world::Entity* JointBindingComponent::getEntity(const std::wstring& name, int32_t index) const
{
	for (const auto& binding : m_bindings)
	{
		if (binding.entity->getName() == name)
		{
			if (index-- <= 0)
				return binding.entity;
		}
	}
	return nullptr;
}

RefArray< world::Entity > JointBindingComponent::getEntities(const std::wstring& name) const
{
	RefArray< world::Entity > entities;
	for (const auto& binding : m_bindings)
	{
		if (binding.entity->getName() == name)
			entities.push_back(binding.entity);
	}
	return entities;
}

}
