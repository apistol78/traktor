#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Scene/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.LayerEntityData", 0, LayerEntityData, world::GroupEntityData)

LayerEntityData::LayerEntityData()
:	m_visible(true)
,	m_locked(false)
,	m_include(true)
{
}

bool LayerEntityData::serialize(ISerializer& s)
{
	if (!world::GroupEntityData::serialize(s))
		return false;

	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"locked", m_locked);
	s >> Member< bool >(L"include", m_include);

	return true;
}

	}
}
