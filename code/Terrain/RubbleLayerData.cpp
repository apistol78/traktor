#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Member.h"
#include "Terrain/RubbleLayer.h"
#include "Terrain/RubbleLayerData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RubbleLayerData", 1, RubbleLayerData, ITerrainLayerData)

RubbleLayerData::RubbleLayerData()
:	m_spreadDistance(100.0f)
{
}

Ref< ITerrainLayer > RubbleLayerData::createLayerInstance(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TerrainComponent& terrainComponent
) const
{
	Ref< RubbleLayer > layer = new RubbleLayer();
	if (layer->create(resourceManager, renderSystem, *this, terrainComponent))
		return layer;
	else
		return 0;
}

void RubbleLayerData::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 1);

	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> MemberStlVector< RubbleMesh, MemberComposite< RubbleMesh > >(L"rubble", m_rubble);
}

RubbleLayerData::RubbleMesh::RubbleMesh()
:	material(1)
,	density(10)
,	randomScaleAmount(0.5f)
{
}

void RubbleLayerData::RubbleMesh::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::InstanceMesh >(L"mesh", mesh);
	s >> Member< uint8_t >(L"material", material);
	s >> Member< int32_t >(L"density", density, AttributeRange(0.0f));
	s >> Member< float >(L"randomScaleAmount", randomScaleAmount, AttributeRange(0.0f, 1.0f));
}

	}
}
