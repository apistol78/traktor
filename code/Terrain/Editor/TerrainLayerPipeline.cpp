#include "Editor/IPipelineDepends.h"
#include "Terrain/RubbleLayerData.h"
#include "Terrain/UndergrowthLayerData.h"
#include "Terrain/Editor/TerrainLayerPipeline.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainLayerPipeline", 0, TerrainLayerPipeline, editor::DefaultPipeline)

TypeInfoSet TerrainLayerPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RubbleLayerData >());
	typeSet.insert(&type_of< UndergrowthLayerData >());
	return typeSet;
}

bool TerrainLayerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const UndergrowthLayerData* undergrowthLayerData = checked_type_cast< const UndergrowthLayerData*, false >(sourceAsset))
		pipelineDepends->addDependency(undergrowthLayerData->m_shader, editor::PdfBuild | editor::PdfResource);
	else if (const RubbleLayerData* rubbleLayerData = checked_type_cast< const RubbleLayerData*, false >(sourceAsset))
		pipelineDepends->addDependency(rubbleLayerData->m_mesh, editor::PdfBuild | editor::PdfResource);
	return true;
}

	}
}
