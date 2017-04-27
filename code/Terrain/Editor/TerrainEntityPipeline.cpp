/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Editor/IPipelineDepends.h"
#include "Terrain/Editor/TerrainEntityPipeline.h"
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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OceanComponentData >());
	typeSet.insert(&type_of< RiverComponentData >());
	typeSet.insert(&type_of< TerrainComponentData >());
	return typeSet;
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

		const RefArray< ITerrainLayerData >& layers = terrainComponentData->getLayers();
		for (RefArray< ITerrainLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			if (const UndergrowthLayerData* undergrowthLayerData = dynamic_type_cast< const UndergrowthLayerData* >(*i))
			{
				pipelineDepends->addDependency(undergrowthLayerData->m_shader, editor::PdfBuild | editor::PdfResource);
			}
			else if (const RubbleLayerData* rubbleLayerData = dynamic_type_cast< const RubbleLayerData* >(*i))
			{
				for (std::vector< RubbleLayerData::RubbleMesh >::const_iterator i = rubbleLayerData->m_rubble.begin(); i != rubbleLayerData->m_rubble.end(); ++i)
				{
					const Guid meshId = i->mesh;
					pipelineDepends->addDependency(meshId, editor::PdfBuild | editor::PdfResource);
				}
			}
		}
	}
	return true;
}

	}
}
