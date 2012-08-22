#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.LayerEntityData", 0, LayerEntityData, world::GroupEntityData)

LayerEntityData::LayerEntityData()
:	m_visible(true)
,	m_locked(false)
,	m_include(true)
,	m_dynamic(false)
{
}

bool LayerEntityData::serialize(ISerializer& s)
{
	if (!world::GroupEntityData::serialize(s))
		return false;

	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"locked", m_locked);
	s >> Member< bool >(L"include", m_include);
	s >> Member< bool >(L"dynamic", m_dynamic);

	return true;
}

	}
}
