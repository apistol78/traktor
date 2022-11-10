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
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/ILayerAttribute.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.LayerEntityData", 3, LayerEntityData, EntityData)

LayerEntityData::LayerEntityData()
{
	setComponent(new EditorAttributesComponentData());
	setComponent(new GroupComponentData());
}

void LayerEntityData::setAttribute(const ILayerAttribute* attribute)
{
	T_ASSERT(attribute);
	for (auto attr = m_attributes.begin(); attr != m_attributes.end(); ++attr)
	{
		if (is_type_a(type_of(*attr), type_of(attribute)))
		{
			*attr = attribute;
			return;
		}
	}
	m_attributes.push_back(attribute);
}

const ILayerAttribute* LayerEntityData::getAttribute(const TypeInfo& attributeType) const
{
	for (auto attr : m_attributes)
	{
		if (is_type_a(type_of(attr), attributeType))
			return attr;
	}
	return nullptr;
}

void LayerEntityData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);

	EntityData::serialize(s);

	if (s.getVersion() < 2)
	{
		RefArray< EntityData > entityData;
		s >> MemberRefArray< EntityData >(L"entityData", entityData, AttributePrivate());
		setComponent(new GroupComponentData(entityData));
	}

	if (s.getVersion() < 3)
	{
		Ref< EditorAttributesComponentData > editorAttributes = new EditorAttributesComponentData();
		s >> Member< bool >(L"visible", editorAttributes->visible);
		s >> Member< bool >(L"locked", editorAttributes->locked);
		s >> Member< bool >(L"include", editorAttributes->include);
		s >> Member< bool >(L"dynamic", editorAttributes->dynamic);
		setComponent(editorAttributes);
	}

	if (s.getVersion() < 3)
		s >> MemberRefArray< const ILayerAttribute >(L"attributes", m_attributes);
}

	}
}
