#include "Editor/IPipelineDepends.h"
#include "Terrain/UndergrowthLayerData.h"
#include "Terrain/Editor/UndergrowthPipeline.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.UndergrowthPipeline", 0, UndergrowthPipeline, editor::DefaultPipeline)

TypeInfoSet UndergrowthPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< UndergrowthLayerData >());
	return typeSet;
}

bool UndergrowthPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const UndergrowthLayerData* undergrowthLayerData = checked_type_cast< const UndergrowthLayerData*, false >(sourceAsset);
	pipelineDepends->addDependency(undergrowthLayerData->m_shader, editor::PdfBuild | editor::PdfResource);
	return true;
}

	}
}
