#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Sound/StaticSoundResource.h"
#include "Sound/StreamSoundResource.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/SoundPipeline.h"
#include "Sound/Editor/Encoders/OggStreamEncoder.h"
#include "Sound/Decoders/FlacStreamDecoder.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"
#include "Sound/Decoders/OggStreamDecoder.h"
#include "Sound/Decoders/WavStreamDecoder.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundPipeline", 28, SoundPipeline, editor::IPipeline)

SoundPipeline::SoundPipeline()
{
}

bool SoundPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void SoundPipeline::destroy()
{
}

TypeInfoSet SoundPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SoundAsset >());
	return typeSet;
}

bool SoundPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, soundAsset->getFileName());
	pipelineDepends->addDependency(fileName);

	Ref< const SoundCategory > category = pipelineDepends->getObjectReadOnly< SoundCategory >(soundAsset->m_category);
	if (category)
		pipelineDepends->addDependency(soundAsset->m_category, editor::PdfUse);

	while (category)
	{
		Ref< const SoundCategory > parent = pipelineDepends->getObjectReadOnly< SoundCategory >(category->getParent());
		if (parent)
			pipelineDepends->addDependency(category->getParent(), editor::PdfUse);

		category = parent;
	}

	return true;
}

bool SoundPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, soundAsset->getFileName());

	Ref< IStreamDecoder > decoder;
	if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"wav") == 0)
		decoder = new sound::WavStreamDecoder();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"flac") == 0)
		decoder = new sound::FlacStreamDecoder();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"mp3") == 0)
		decoder = new sound::Mp3StreamDecoder();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"ogg") == 0)
		decoder = new sound::OggStreamDecoder();
	else
	{
		log::error << L"Failed to build sound asset, unable to determine decoder from extension" << Endl;
		return false;
	}

	Ref< IStream > sourceStream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to build sound asset, unable to open source" << Endl;
		return false;
	}

	float volume = 1.0f;
	float presence = soundAsset->m_presence;
	float presenceRate = soundAsset->m_presenceRate;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(soundAsset->m_category);
	while (category)
	{
		volume *= category->getVolume();

		if (presence <= FUZZY_EPSILON)
		{
			presence = category->getPresence();
			presenceRate = category->getPresenceRate();
		}

		range = max(range, category->getRange());

		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	log::info << L"Category volume " << int32_t(volume * 100.0f) << L" %" << Endl;
	log::info << L"Category presence " << presence << L", rate " << int32_t(presenceRate * 100.0f) << L" d%" << Endl;
	log::info << L"Category range " << range << Endl;

	if (soundAsset->m_stream)
	{
		Ref< StreamSoundResource > resource = new StreamSoundResource();

		resource->m_decoderType = &type_of(decoder);
		resource->m_volume = volume;
		resource->m_presence = presence;
		resource->m_presenceRate = presenceRate;
		resource->m_range = range;
		resource->m_preload = soundAsset->m_preload;

		Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
			outputPath,
			outputGuid
		);
		if (!instance)
		{
			log::error << L"Failed to build sound asset, unable to create instance" << Endl;
			return false;
		}

		instance->setObject(resource);

		Ref< IStream > stream = instance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Failed to build sound asset, unable to create data stream" << Endl;
			instance->revert();
			return false;
		}

		int32_t dataOffsetBegin = stream->tell();

		bool result = StreamCopy(stream, sourceStream).execute();

		int32_t dataOffsetEnd = stream->tell();

		stream->close();
		sourceStream->close();

		if (result && !instance->commit())
		{
			log::error << L"Failed to build sound asset, unable to commit instance" << Endl;
			return false;
		}
		else if (!result)
		{
			log::error << L"Failed to build sound asset, unable to copy source data" << Endl;
			return false;
		}

		// Create report.
		Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Sound", outputGuid);
		if (report)
		{
			report->set(L"path", outputPath);
			report->set(L"type", 0);
			report->set(L"preload", soundAsset->m_preload);
			report->set(L"sampleRate", 0);
			report->set(L"samplesCount", 0);
			report->set(L"channelsCount", 0);
			report->set(L"size", dataOffsetEnd - dataOffsetBegin);
		}
	}
	else
	{
		Ref< StaticSoundResource > resource = new StaticSoundResource();

		Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
			outputPath,
			outputGuid
		);
		if (!instance)
		{
			log::error << L"Failed to build sound asset, unable to create instance" << Endl;
			return false;
		}

		Ref< IStream > stream = instance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Failed to build sound asset, unable to create data stream" << Endl;
			instance->revert();
			return false;
		}

		int32_t dataOffsetBegin = stream->tell();

		// Prepare decoder with source stream.
		if (!decoder->create(sourceStream))
		{
			log::error << L"Failed to build sound asset, unable to create stream decoder" << Endl;
			return false;
		}

		// Prepare encoder with destination stream.
		Ref< IStreamEncoder > encoder = new OggStreamEncoder();
		if (!encoder->create(stream))
		{
			log::error << L"Failed to build sound asset, unable to create stream encoder" << Endl;
			return false;
		}

		// Transcode source stream.
		uint32_t sampleRate = 0;
		uint32_t samplesCount = 0;
		uint32_t maxChannel = 0;

		SoundBlock soundBlock;
		std::memset(&soundBlock, 0, sizeof(soundBlock));
		soundBlock.samplesCount = 4096;

		while (decoder->getBlock(soundBlock))
		{
			if (soundBlock.samplesCount > 0 && soundBlock.maxChannel > 0)
			{
				if (!encoder->putBlock(soundBlock))
				{
					log::error << L"Failed to build sound asset, transcoding failed" << Endl;
					return false;
				}
			}

			sampleRate = soundBlock.sampleRate;
			samplesCount += soundBlock.samplesCount;
			maxChannel = std::max(soundBlock.maxChannel, maxChannel);

			std::memset(&soundBlock, 0, sizeof(soundBlock));
			soundBlock.samplesCount = 4096;
		}

		safeDestroy(encoder);
		safeDestroy(decoder);

		sourceStream->close();

		resource->m_sampleRate = sampleRate;
		resource->m_samplesCount = samplesCount;
		resource->m_channelsCount = maxChannel;
		resource->m_volume = volume;
		resource->m_presence = presence;
		resource->m_presenceRate = presenceRate;
		resource->m_range = range;
		resource->m_decoderType = &type_of< OggStreamDecoder >();

		int32_t dataOffsetEnd = stream->tell();
		stream->close();

		instance->setObject(resource);

		if (!instance->commit())
		{
			log::error << L"Failed to build sound asset, unable to commit instance" << Endl;
			return false;
		}

		// Create report.
		Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Sound", outputGuid);
		if (report)
		{
			report->set(L"path", outputPath);
			report->set(L"type", 1);
			report->set(L"preload", 1);
			report->set(L"sampleRate", sampleRate);
			report->set(L"samplesCount", samplesCount);
			report->set(L"channelsCount", maxChannel);
			report->set(L"size", dataOffsetEnd - dataOffsetBegin);
		}
	}

	return true;
}

Ref< ISerializable > SoundPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
