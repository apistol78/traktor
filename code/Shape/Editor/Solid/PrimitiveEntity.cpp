/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrimitiveEntity", PrimitiveEntity, world::Entity)

PrimitiveEntity::PrimitiveEntity(const PrimitiveEntityData* data, const Transform& transform)
:	world::Entity(data->getId(), data->getName(), transform)
,	m_data(data)
,   m_selectedMaterial(model::c_InvalidIndex)
,   m_dirty(true)
{
}

void PrimitiveEntity::setTransform(const Transform& transform)
{
	if (getTransform() != transform)
	{
		world::Entity::setTransform(transform);
		m_dirty = true;
	}
}

Aabb3 PrimitiveEntity::getBoundingBox() const
{
	return m_boundingBox;
}

void PrimitiveEntity::update(const world::UpdateParams& update)
{
}

}
