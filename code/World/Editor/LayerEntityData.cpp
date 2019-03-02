#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Editor/ILayerAttribute.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.LayerEntityData", 1, LayerEntityData, world::GroupEntityData)

LayerEntityData::LayerEntityData()
:	m_visible(true)
,	m_locked(false)
,	m_include(true)
,	m_dynamic(false)
{
}

void LayerEntityData::setAttribute(const ILayerAttribute* attribute)
{
	T_ASSERT (attribute);
	for (RefArray< const ILayerAttribute >::iterator i = m_attributes.begin(); i != m_attributes.end(); ++i)
	{
		if (is_type_a(type_of(*i), type_of(attribute)))
		{
			*i = attribute;
			return;
		}
	}
	m_attributes.push_back(attribute);
}

const ILayerAttribute* LayerEntityData::getAttribute(const TypeInfo& attributeType) const
{
	for (RefArray< const ILayerAttribute >::const_iterator i = m_attributes.begin(); i != m_attributes.end(); ++i)
	{
		if (is_type_a(type_of(*i), attributeType))
			return *i;
	}
	return 0;
}

void LayerEntityData::serialize(ISerializer& s)
{
	world::GroupEntityData::serialize(s);

	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"locked", m_locked);
	s >> Member< bool >(L"include", m_include);
	s >> Member< bool >(L"dynamic", m_dynamic);

	if (s.getVersion() >= 1)
		s >> MemberRefArray< const ILayerAttribute >(L"attributes", m_attributes);
}

	}
}
