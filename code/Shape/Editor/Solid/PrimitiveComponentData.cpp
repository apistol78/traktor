/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Shape/Editor/Solid/PrimitiveComponent.h"
#include "Shape/Editor/Solid/PrimitiveComponentData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrimitiveComponentData", 0, PrimitiveComponentData, world::IEntityComponentData)

Ref< PrimitiveComponent > PrimitiveComponentData::createComponent() const
{
	Ref< PrimitiveComponent > component = new PrimitiveComponent(this);
	if (m_shape)
	{
		Ref< const model::Model > m = m_shape->createModel();
		if (!m)
			return nullptr;

		component->m_model = m;
		component->m_boundingBox = m->getBoundingBox();
	}
	return component;
}

void PrimitiveComponentData::setMaterial(int32_t face, const Guid& material)
{
	if (material.isNotNull())
		m_materials[face] = material;
	else
		m_materials.remove(face);
}

int32_t PrimitiveComponentData::getOrdinal() const
{
	return 0;
}

void PrimitiveComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PrimitiveComponentData::serialize(ISerializer& s)
{
	const MemberEnum< BooleanOperation >::Key c_BooleanOperation_Keys[] =
	{
		{ L"Union", BooleanOperation::Union },
		{ L"Intersection", BooleanOperation::Intersection },
		{ L"Difference", BooleanOperation::Difference },
		{ 0 }
	};

	s >> MemberEnum< BooleanOperation >(L"operation", m_operation, c_BooleanOperation_Keys);
	s >> MemberRef< IShape >(L"shape", m_shape);
	s >> MemberSmallMap< int32_t, Guid >(L"materials", m_materials, AttributePrivate());
}

}
