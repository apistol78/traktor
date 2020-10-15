#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Editor/ILayerAttribute.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.LayerEntityData", 2, LayerEntityData, EntityData)

LayerEntityData::LayerEntityData()
{
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

	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"locked", m_locked);
	s >> Member< bool >(L"include", m_include);
	s >> Member< bool >(L"dynamic", m_dynamic);

	s >> MemberRefArray< const ILayerAttribute >(L"attributes", m_attributes);
}

	}
}
