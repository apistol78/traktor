#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/Editor/OcclusionTextureAsset.h"
#include "Mesh/Editor/OcclusionTexturePipeline.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Model/Formats/ModelFormat.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.OcclusionTexturePipeline", 0, OcclusionTexturePipeline, editor::DefaultPipeline)

bool OcclusionTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet OcclusionTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OcclusionTextureAsset >());
	return typeSet;
}

bool OcclusionTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, asset->getFileName());
	pipelineDepends->addDependency(fileName);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool OcclusionTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, asset->getFileName());

	if (!asset->m_output.m_scaleImage || asset->m_output.m_scaleWidth <= 0 || asset->m_output.m_scaleHeight <= 0)
	{
		log::error << L"Occlusion texture pipeline failed; must define a scale size of occlusion texture asset" << Endl;
		return 0;
	}

	// Read source model.
	Ref< model::Model > model = model::ModelFormat::readAny(fileName);
	if (!model)
	{
		log::error << L"Occlusion texture pipeline failed; unable to read source model (" << fileName.getPathName() << L")" << Endl;
		return 0;
	}

	// Ensure model is triangulated.
	model::triangulateModel(*model);

	// Bake occlusion map.
	Ref< drawing::Image > image = model::bakePixelOcclusion(*model, asset->m_output.m_scaleWidth, asset->m_output.m_scaleHeight);
	if (!image)
	{
		log::error << L"Occlusion texture pipeline failed; unable to bake occlusion map" << Endl;
		return 0;
	}

	// Run image through texture pipeline for compression etc.
	return pipelineBuilder->buildOutput(
		&asset->m_output,
		image,
		outputPath,
		outputGuid
	);
}

	}
}
