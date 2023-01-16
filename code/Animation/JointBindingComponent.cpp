/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
		m_transform = m_owner->getTransform();
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
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto& binding : m_bindings)
	{
		Aabb3 childBoundingBox = binding.entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			const Transform childTransform = binding.entity->getTransform();
			const Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void JointBindingComponent::update(const world::UpdateParams& update)
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (skeletonComponent)
	{
		for (const auto& binding : m_bindings)
		{
			Transform T;
			if (skeletonComponent->getPoseTransform(binding.jointHandle, T))
				binding.entity->setTransform(m_transform * T);
		}
	}

	for (const auto& binding : m_bindings)
		binding.entity->update(update);
}

}
