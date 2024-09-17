/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Shape/Editor/Solid/PrimitiveComponent.h"
#include "Shape/Editor/Solid/PrimitiveComponentData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrimitiveComponent", PrimitiveComponent, world::IEntityComponent)

PrimitiveComponent::PrimitiveComponent(const PrimitiveComponentData* data)
:	m_data(data)
,   m_selectedMaterial(model::c_InvalidIndex)
,	m_transform(Transform::identity())
,   m_dirty(true)
{
}

void PrimitiveComponent::destroy()
{
}

void PrimitiveComponent::setOwner(world::Entity* owner)
{
}

void PrimitiveComponent::setTransform(const Transform& transform)
{
	if (m_transform != transform)
	{
		m_transform = transform;
		m_dirty = true;
	}
}

Aabb3 PrimitiveComponent::getBoundingBox() const
{
	return m_boundingBox;
}

void PrimitiveComponent::update(const world::UpdateParams& update)
{
}

}
