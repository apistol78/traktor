/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/OrientateComponent.h"
#include "Core/Math/Const.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.OrientateComponent", OrientateComponent, world::IEntityComponent)

void OrientateComponent::destroy()
{
	m_owner = nullptr;
}

void OrientateComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void OrientateComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 OrientateComponent::getBoundingBox() const
{
	return Aabb3();
}

void OrientateComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	if (update.deltaTime <= FUZZY_EPSILON)
	{
		m_owner->setTransform(m_transform);
		return;
	}

	Vector4 direction = m_transform.translation() - m_lastTransform.translation();
	const Scalar ln = direction.length();
	if (ln <= FUZZY_EPSILON)
		return;

	const Matrix44 Mlook = lookAt(
		m_lastTransform.translation(),
		m_transform.translation()
	);

	const Transform Tlook(Mlook);

	m_lastTransform = m_transform;
	m_transform = Transform(m_transform.translation(), Tlook.rotation().inverse());

	m_owner->setTransform(m_transform);
}

}
