#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Terrain/UndergrowthPlant.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.UndergrowthPlant", 0, UndergrowthPlant, ISerializable)

void UndergrowthPlant::serialize(ISerializer& s)
{
	s >> MemberStlVector< int32_t >(L"plants", m_plants);
}

	}
}
