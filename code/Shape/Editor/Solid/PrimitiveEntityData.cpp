/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrimitiveEntityData", 1, PrimitiveEntityData, world::EntityData)

PrimitiveEntityData::PrimitiveEntityData()
:   m_operation(BooleanOperation::Union)
{
}

Ref< PrimitiveEntity > PrimitiveEntityData::createEntity() const
{
	Ref< PrimitiveEntity > entity = new PrimitiveEntity(this, getTransform());
	if (m_shape)
	{
		Ref< const model::Model > m = m_shape->createModel();
		if (!m)
			return nullptr;

		entity->m_model = m;
		entity->m_boundingBox = m->getBoundingBox();
	}
	return entity;
}

void PrimitiveEntityData::setMaterial(int32_t face, const Guid& material)
{
	if (material.isNotNull())
		m_materials[face] = material;
	else
		m_materials.remove(face);
}

void PrimitiveEntityData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< PrimitiveEntityData >() >= 1);

	const MemberEnum< BooleanOperation >::Key c_BooleanOperation_Keys[] =
	{
		{ L"Union", BooleanOperation::Union },
		{ L"Intersection", BooleanOperation::Intersection },
		{ L"Difference", BooleanOperation::Difference },
		{ 0 }
	};

	world::EntityData::serialize(s);

	s >> MemberEnum< BooleanOperation >(L"operation", m_operation, c_BooleanOperation_Keys);
	s >> MemberRef< IShape >(L"shape", m_shape);
	s >> MemberSmallMap< int32_t, Guid >(L"materials", m_materials, AttributePrivate());
}

	}
}