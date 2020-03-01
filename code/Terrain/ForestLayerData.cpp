#include "Core/Serialization/ISerializer.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Member.h"
#include "Terrain/ForestLayer.h"
#include "Terrain/ForestLayerData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.ForestLayerData", 0, ForestLayerData, ITerrainLayerData)

ForestLayerData::ForestLayerData()
{
}

Ref< ITerrainLayer > ForestLayerData::createLayerInstance(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TerrainComponent& terrainComponent
) const
{
	Ref< ForestLayer > layer = new ForestLayer();
	if (layer->create(resourceManager, renderSystem, *this, terrainComponent))
		return layer;
	else
		return nullptr;
}

void ForestLayerData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::InstanceMesh >(L"lod0mesh", m_lod0mesh);
	s >> resource::Member< mesh::InstanceMesh >(L"lod1mesh", m_lod1mesh);
}

	}
}
