#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Member.h"
#include "Terrain/ForestComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.ForestComponentData", 2, ForestComponentData, TerrainLayerComponentData)

void ForestComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::InstanceMesh >(L"lod0mesh", m_lod0mesh);
	s >> resource::Member< mesh::InstanceMesh >(L"lod1mesh", m_lod1mesh);

	if (s.getVersion< ForestComponentData >() >= 2)
		s >> Member< uint8_t >(L"attribute", m_attribute);
	else
		s >> Member< uint8_t >(L"material", m_attribute);

	s >> Member< float >(L"density", m_density, AttributeRange(0.0f));
	s >> Member< float >(L"lod0distance", m_lod0distance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"lod1distance", m_lod1distance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"upness", m_upness, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"randomScale", m_randomScale, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"randomTilt", m_randomTilt, AttributeRange(0.0f, HALF_PI) | AttributeUnit(UnitType::Radians));

	if (s.getVersion< ForestComponentData >() >= 1)
		s >> Member< float >(L"slopeAngleThreshold", m_slopeAngleThreshold, AttributeRange(0.0f, HALF_PI) | AttributeUnit(UnitType::Radians));
}

}
