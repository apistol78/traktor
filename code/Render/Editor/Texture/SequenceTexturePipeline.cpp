#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/SequenceTextureAsset.h"
#include "Render/Editor/Texture/SequenceTexturePipeline.h"
#include "Render/Resource/SequenceTextureResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SequenceTexturePipeline", 0, SequenceTexturePipeline, editor::DefaultPipeline)

bool SequenceTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet SequenceTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SequenceTextureAsset >();
	return typeSet;
}

bool SequenceTexturePipeline::shouldCache() const
{
	return true;
}

bool SequenceTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SequenceTextureAsset* asset = checked_type_cast< const SequenceTextureAsset* >(sourceAsset);

	for (const auto& p : asset->m_fileNames)
		pipelineDepends->addDependency(Path(m_assetPath), p.getOriginal());

	pipelineDepends->addDependency< SequenceTextureAsset >();
	pipelineDepends->addDependency< TextureOutput >();

	return true;
}

bool SequenceTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const SequenceTextureAsset* asset = checked_type_cast< const SequenceTextureAsset* >(sourceAsset);

	Ref< SequenceTextureResource > resource = new SequenceTextureResource();
	resource->m_rate = asset->m_rate;

	Guid frameOutputGuid = outputGuid.permutation(1);

	for (const auto& p : asset->m_fileNames)
	{
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + p);
		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
		if (!file)
		{
			log::error << L"Sequence texture asset pipeline failed; unable to open source image \"" << p.getOriginal() << L"\"." << Endl;
			return false;
		}

		Ref< drawing::Image > image = drawing::Image::load(file, p.getExtension());
		if (!image)
		{
			log::error << L"Sequence texture asset pipeline failed; unable to load source image \"" << p.getOriginal() << L"\"." << Endl;
			return false;
		}

		file->close();

		std::wstring frameOutputPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/" + frameOutputGuid.format();

		Ref< TextureOutput > output = new TextureOutput(asset->m_output);
		if (!pipelineBuilder->buildAdHocOutput(
			output,
			frameOutputPath,
			frameOutputGuid,
			image
		))
		{
			log::error << L"Sequence texture asset pipeline failed; unable to build source image \"" << p.getOriginal() << L"\"" << Endl;
			return false;
		}

		resource->m_textures.push_back(resource::Id< ITexture >(
			frameOutputGuid
		));

		frameOutputGuid = frameOutputGuid.permutation(1);
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Sequence texture asset pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(resource);

	if (!outputInstance->commit())
	{
		log::error << L"Sequence texture asset pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
