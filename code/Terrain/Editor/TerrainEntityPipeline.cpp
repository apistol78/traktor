#include "Editor/IPipelineDepends.h"
#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/ForestLayerData.h"
#include "Terrain/ITerrainLayerData.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/RiverComponentData.h"
#include "Terrain/RubbleLayerData.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/UndergrowthLayerData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEntityPipeline", 0, TerrainEntityPipeline, world::EntityPipeline)

TypeInfoSet TerrainEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		OceanComponentData,
		RiverComponentData,
		TerrainComponentData
	>();
}

bool TerrainEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const OceanComponentData* oceanComponentData = dynamic_type_cast< const OceanComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(oceanComponentData->getReflectionMap(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(oceanComponentData->getTerrain(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const RiverComponentData* riverComponentData = dynamic_type_cast< const RiverComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(riverComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const TerrainComponentData* terrainComponentData = dynamic_type_cast< const TerrainComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(terrainComponentData->getTerrain(), editor::PdfBuild | editor::PdfResource);
		for (auto layer : terrainComponentData->getLayers())
		{
			if (const ForestLayerData* forestLayerData = dynamic_type_cast< const ForestLayerData* >(layer))
			{
				pipelineDepends->addDependency(forestLayerData->m_lod0mesh, editor::PdfBuild | editor::PdfResource);
				pipelineDepends->addDependency(forestLayerData->m_lod1mesh, editor::PdfBuild | editor::PdfResource);
			}
			else if (const RubbleLayerData* rubbleLayerData = dynamic_type_cast< const RubbleLayerData* >(layer))
			{
				for (const auto& rubble : rubbleLayerData->m_rubble)
					pipelineDepends->addDependency(rubble.mesh, editor::PdfBuild | editor::PdfResource);
			}
			else if (const UndergrowthLayerData* undergrowthLayerData = dynamic_type_cast< const UndergrowthLayerData* >(layer))
			{
				pipelineDepends->addDependency(undergrowthLayerData->m_shader, editor::PdfBuild | editor::PdfResource);
			}
		}
	}
	return true;
}

	}
}
