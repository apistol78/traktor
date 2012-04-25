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
		namespace
		{

const Guid c_guidTerrainShaderVFetch(L"{480B7C64-5494-A74A-8485-F2CA15A900E6}");
const Guid c_guidTerrainShaderStatic(L"{557537A0-F1E3-AF4C-ACB9-FD862FC3265C}");

		}

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
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	if (const TerrainEntityData* terrainEntityData = dynamic_type_cast< const TerrainEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(c_guidTerrainShaderVFetch, editor::PdfBuild);
		pipelineDepends->addDependency(c_guidTerrainShaderStatic, editor::PdfBuild);
		pipelineDepends->addDependency(terrainEntityData->getTerrain(), editor::PdfBuild);
	}
	else if (const OceanEntityData* oceanEntityData = dynamic_type_cast< const OceanEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanEntityData->getHeightfield(), editor::PdfBuild);
		pipelineDepends->addDependency(oceanEntityData->getShader(), editor::PdfBuild);
	}
	else if (const RiverEntityData* riverEntityData = dynamic_type_cast< const RiverEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(riverEntityData->getShader(), editor::PdfBuild);
	}
	else if (const UndergrowthEntityData* undergrowthEntityData = dynamic_type_cast< const UndergrowthEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(undergrowthEntityData->getHeightfield(), editor::PdfBuild);
		pipelineDepends->addDependency(undergrowthEntityData->getMaterialMask(), editor::PdfBuild);
		pipelineDepends->addDependency(undergrowthEntityData->getShader(), editor::PdfBuild);
	}
	return true;
}

	}
}
