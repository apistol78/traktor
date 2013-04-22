#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/RiverEntityData.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEntityPipeline", 0, TerrainEntityPipeline, world::EntityPipeline)

TypeInfoSet TerrainEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< RiverEntityData >());
	typeSet.insert(&type_of< UndergrowthEntityData >());
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
	if (const TerrainEntityData* terrainEntityData = dynamic_type_cast< const TerrainEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(terrainEntityData->getTerrain(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const OceanEntityData* oceanEntityData = dynamic_type_cast< const OceanEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanEntityData->getShaderComposite(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const RiverEntityData* riverEntityData = dynamic_type_cast< const RiverEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(riverEntityData->getShader(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const UndergrowthEntityData* undergrowthEntityData = dynamic_type_cast< const UndergrowthEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(undergrowthEntityData->getTerrain(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(undergrowthEntityData->getMaterialMask(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(undergrowthEntityData->getShader(), editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

	}
}
