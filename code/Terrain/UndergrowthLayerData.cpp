#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/UndergrowthLayer.h"
#include "Terrain/UndergrowthLayerData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.UndergrowthLayerData", 0, UndergrowthLayerData, ITerrainLayerData)

UndergrowthLayerData::UndergrowthLayerData()
:	m_spreadDistance(100.0f)
{
}

Ref< ITerrainLayer > UndergrowthLayerData::createLayerInstance(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TerrainComponent& terrainComponent
) const
{
	Ref< UndergrowthLayer > layer = new UndergrowthLayer();
	if (layer->create(resourceManager, renderSystem, *this, terrainComponent))
		return layer;
	else
		return 0;
}

void UndergrowthLayerData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> MemberStlVector< Plant, MemberComposite< Plant > >(L"plants", m_plants);
}

UndergrowthLayerData::Plant::Plant()
:	material(1)
,	density(100)
,	plant(0)
,	scale(1.0f)
{
}

void UndergrowthLayerData::Plant::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"material", material);
	s >> Member< int32_t >(L"density", density);
	s >> Member< int32_t >(L"plant", plant);
	s >> Member< float >(L"scale", scale);
}

	}
}
