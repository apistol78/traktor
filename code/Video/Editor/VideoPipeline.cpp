/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Video/Editor/VideoPipeline.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Video/Editor/VideoAsset.h"
#include "Video/VideoResource.h"

namespace traktor::video
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoPipeline", 0, VideoPipeline, editor::IPipeline)

bool VideoPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void VideoPipeline::destroy()
{
}

TypeInfoSet VideoPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< VideoAsset >();
}

bool VideoPipeline::shouldCache() const
{
	return false;
}

uint32_t VideoPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool VideoPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	const VideoAsset* videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), videoAsset->getFileName().getOriginal());
	return true;
}

bool VideoPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason) const
{
	const VideoAsset* videoAsset = checked_type_cast< const VideoAsset* >(sourceAsset);

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + videoAsset->getFileName());
	Ref< IStream > sourceStream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to build video asset, unable to open source." << Endl;
		return false;
	}

	Ref< VideoResource > resource = new VideoResource();

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!instance)
	{
		log::error << L"Failed to build video asset, unable to create instance." << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build video asset, unable to create data stream." << Endl;
		instance->revert();
		return false;
	}

	bool result = StreamCopy(stream, sourceStream).execute();

	stream->close();
	sourceStream->close();

	if (result && !instance->commit())
	{
		log::error << L"Failed to build video asset, unable to commit instance." << Endl;
		return false;
	}
	else if (!result)
	{
		log::error << L"Failed to build video asset, unable to copy source data." << Endl;
		instance->revert();
		return false;
	}

	return true;
}

Ref< ISerializable > VideoPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
