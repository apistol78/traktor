/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/RotatorComponent.h"
#include "Core/Math/Const.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RotatorComponent", RotatorComponent, world::IEntityComponent)

RotatorComponent::RotatorComponent(Axis axis, float rate)
:	m_axis(axis)
,	m_rate(rate)
{
}

void RotatorComponent::destroy()
{
	m_owner = nullptr;
}

void RotatorComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void RotatorComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 RotatorComponent::getBoundingBox() const
{
	return Aabb3();
}

void RotatorComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	if (update.deltaTime <= FUZZY_EPSILON)
	{
		m_owner->setTransform(m_transform);
		return;
	}

	const float angle = m_rate * update.totalTime;

	switch (m_axis)
	{
	case Axis::X:
		m_local = Transform(rotateX(angle));
		break;

	case Axis::Y:
		m_local = Transform(rotateY(angle));
		break;

	case Axis::Z:
		m_local = Transform(rotateZ(angle));
		break;

	default:
		m_local = Transform::identity();
		break;
	}

	m_owner->setTransform(m_transform * m_local);
}

}
