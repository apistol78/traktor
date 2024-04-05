/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/ControlPointComponent.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponent", ControlPointComponent, world::IEntityComponent)

ControlPointComponent::ControlPointComponent(const ControlPointComponentData* data)
:	m_dirty(true)
,	m_transform(Transform::identity())
,	m_data(data)
{
}

void ControlPointComponent::destroy()
{
}

void ControlPointComponent::setOwner(world::Entity* owner)
{
}

void ControlPointComponent::setTransform(const Transform& transform)
{
	if (transform != m_transform)
	{
		m_dirty = true;
		m_transform = transform;
	}
}

Aabb3 ControlPointComponent::getBoundingBox() const
{
	const Aabb3 boundingBox(
		Vector4(-1.0f, -1.0f, -1.0f),
		Vector4( 1.0f,  1.0f,  1.0f)
	);
	return boundingBox;
}

void ControlPointComponent::update(const world::UpdateParams& update)
{
}

bool ControlPointComponent::checkDirty()
{
	const bool dirty = m_dirty;
	m_dirty = false;
	return dirty;
}

}
