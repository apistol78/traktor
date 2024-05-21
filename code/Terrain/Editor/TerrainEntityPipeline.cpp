/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IPipelineDepends.h"
#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/ForestComponentData.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/RiverComponentData.h"
#include "Terrain/RubbleComponentData.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/UndergrowthComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEntityPipeline", 0, TerrainEntityPipeline, world::EntityPipeline)

TypeInfoSet TerrainEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ForestComponentData >();
	typeSet.insert< OceanComponentData >();
	typeSet.insert< RiverComponentData >();
	typeSet.insert< RubbleComponentData >();
	typeSet.insert< TerrainComponentData >();
	typeSet.insert< UndergrowthComponentData >();
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
	if (const ForestComponentData* forestComponentData = dynamic_type_cast< const ForestComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(forestComponentData->m_lod0mesh, editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(forestComponentData->m_lod1mesh, editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(forestComponentData->m_lod2mesh, editor::PdfBuild | editor::PdfResource);
	}
	else if (const OceanComponentData* oceanComponentData = dynamic_type_cast< const OceanComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanComponentData->getShaderWave(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(oceanComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(oceanComponentData->getTerrain(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const RiverComponentData* riverComponentData = dynamic_type_cast< const RiverComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(riverComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const RubbleComponentData* rubbleComponentData = dynamic_type_cast< const RubbleComponentData* >(sourceAsset))
	{
		for (const auto& rubble : rubbleComponentData->m_rubble)
			pipelineDepends->addDependency(rubble.mesh, editor::PdfBuild | editor::PdfResource);
	}
	else if (const TerrainComponentData* terrainComponentData = dynamic_type_cast< const TerrainComponentData* >(sourceAsset))
	{
		const Guid c_shaderCull(L"{8BA73DD8-0FD9-4C15-A772-EACC14014AEC}");
		pipelineDepends->addDependency(c_shaderCull, editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(terrainComponentData->getTerrain(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const UndergrowthComponentData* undergrowthComponentData = dynamic_type_cast< const UndergrowthComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(undergrowthComponentData->m_shader, editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

}
