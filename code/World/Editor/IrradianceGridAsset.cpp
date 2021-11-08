#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Editor/IrradianceGridAsset.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.IrradianceGridAsset", 1, IrradianceGridAsset, editor::Asset)

IrradianceGridAsset::IrradianceGridAsset()
:	m_intensity(1.0f)
{
}

void IrradianceGridAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion< IrradianceGridAsset >() >= 1)
		s >> Member< float >(L"intensity", m_intensity, AttributeUnit(UnitType::Percent));
}

	}
}
