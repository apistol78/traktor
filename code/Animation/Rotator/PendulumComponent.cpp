/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/PendulumComponent.h"
#include "Core/Math/Const.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PendulumComponent", PendulumComponent, world::IEntityComponent)

PendulumComponent::PendulumComponent(Axis axis, const Vector4& pivot, float amplitude, float rate)
:	m_axis(axis)
,   m_pivot(pivot)
,	m_amplitude(amplitude)
,   m_rate(rate)
{
}

void PendulumComponent::destroy()
{
	m_owner = nullptr;
}

void PendulumComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void PendulumComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 PendulumComponent::getBoundingBox() const
{
	return Aabb3();
}

void PendulumComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	if (update.deltaTime <= FUZZY_EPSILON)
	{
		m_owner->setTransform(m_transform);
		return;
	}

	const float angle = sin(update.totalTime * m_rate) * m_amplitude;

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

    const Transform Tpivot(m_pivot);

	m_owner->setTransform(m_transform * Tpivot * m_local * Tpivot.inverse());
}

}
