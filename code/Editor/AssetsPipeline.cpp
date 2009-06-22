#include "Editor/AssetsPipeline.h"
#include "Editor/Assets.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.AssetsPipeline", AssetsPipeline, Pipeline)

bool AssetsPipeline::create(const Settings* settings)
{
	return true;
}

void AssetsPipeline::destroy()
{
}

uint32_t AssetsPipeline::getVersion() const
{
	return 1;
}

TypeSet AssetsPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Assets >());
	return typeSet;
}

bool AssetsPipeline::buildDependencies(
	PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const Assets* assets = checked_type_cast< const Assets* >(sourceAsset);
	for (std::vector< Guid >::const_iterator i = assets->m_dependencies.begin(); i != assets->m_dependencies.end(); ++i)
		pipelineManager->addDependency(*i);
	return true;
}

bool AssetsPipeline::buildOutput(
	PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	return true;
}

	}
}
