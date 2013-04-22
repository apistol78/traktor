#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
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
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const VideoAsset* videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), videoAsset->getFileName().getOriginal());
	return true;
}

bool VideoPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
	const VideoAsset* videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);

	Ref< IStream > sourceStream = pipelineBuilder->openFile(Path(m_assetPath), videoAsset->getFileName().getOriginal());
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

	bool result = StreamCopy(stream, sourceStream).execute();

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
		instance->revert();
		return false;
	}

	return true;
}

Ref< ISerializable > VideoPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
