#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
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
:	m_density(1000)
,	m_spreadDistance(30.0f)
,	m_cellRadius(20.0f)
,	m_plantScale(1.0f)
{
}

Ref< ITerrainLayer > UndergrowthLayerData::createLayerInstance(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TerrainEntity& terrainEntity
) const
{
	Ref< UndergrowthLayer > layer = new UndergrowthLayer();

	if (!layer->create(resourceManager, renderSystem, *this, terrainEntity))
		return 0;

	return layer;
}

void UndergrowthLayerData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< int32_t >(L"density", m_density);
	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> Member< float >(L"cellRadius", m_cellRadius);
	s >> Member< float >(L"plantScale", m_plantScale);
}

	}
}
