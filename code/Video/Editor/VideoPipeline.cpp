#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Video/VideoResource.h"
#include "Video/Editor/VideoAsset.h"
#include "Video/Editor/VideoPipeline.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoPipeline", 0, VideoPipeline, editor::IPipeline)

bool VideoPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void VideoPipeline::destroy()
{
}

TypeInfoSet VideoPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< VideoAsset >());
	return typeSet;
}

bool VideoPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const VideoAsset > videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, videoAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool VideoPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const VideoAsset > videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, videoAsset->getFileName());

	Ref< IStream > sourceStream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to build video asset, unable to open source" << Endl;
		return false;
	}

	Ref< VideoResource > resource = new VideoResource();

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build video asset, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build video asset, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	// Copy source stream content.
	bool result = true;
	while (sourceStream->available() > 0 && result)
	{
		uint8_t block[1024];
		int readBytes = sourceStream->read(block, sizeof(block));
		if (!stream->write(block, readBytes))
			result = false;
	}

	stream->close();
	sourceStream->close();

	if (result && !instance->commit())
	{
		log::error << L"Failed to build video asset, unable to commit instance" << Endl;
		return false;
	}
	else if (!result)
	{
		log::error << L"Failed to build video asset, unable to copy source data" << Endl;
		return false;
	}

	return true;
}

	}
}
