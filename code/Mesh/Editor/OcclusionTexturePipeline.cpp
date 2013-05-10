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
#include "Model/ModelFormat.h"
#include "Model/Operations/BakePixelOcclusion.h"
#include "Model/Operations/Triangulate.h"

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
	const Guid& outputGuid
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool OcclusionTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const OcclusionTextureAsset* asset = checked_type_cast< const OcclusionTextureAsset* >(sourceAsset);
	if (!asset->m_output.m_scaleImage || asset->m_output.m_scaleWidth <= 0 || asset->m_output.m_scaleHeight <= 0)
	{
		log::error << L"Occlusion texture pipeline failed; must define a scale size of occlusion texture asset" << Endl;
		return 0;
	}

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), asset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Occlusion texture asset pipeline failed; unable to open source model \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	// Read source model.
	Ref< model::Model > model = model::ModelFormat::readAny(file, asset->getFileName().getExtension());
	if (!model)
	{
		log::error << L"Occlusion texture pipeline failed; unable to read source model (" << asset->getFileName().getOriginal() << L")" << Endl;
		return 0;
	}

	// Ensure model is triangulated.
	model::Triangulate().apply(*model);

	// Bake occlusion map.
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getX8R8G8B8(), asset->m_output.m_scaleWidth, asset->m_output.m_scaleHeight);
	if (!model::BakePixelOcclusion(image).apply(*model))
	{
		log::error << L"Occlusion texture pipeline failed; unable to bake occlusion map" << Endl;
		return 0;
	}

	// Run image through texture pipeline for compression etc.
	Ref< render::TextureOutput > output = new render::TextureOutput(asset->m_output);
	return pipelineBuilder->buildOutput(
		output,
		outputPath,
		outputGuid,
		image
	);
}

	}
}
