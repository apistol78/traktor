/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
		namespace
		{

const float c_isMuteThreshold = 16.0f / 32767.0f;

bool isMute(const SoundBlock& soundBlock, uint32_t& outMuteOffset)
{
	for (uint32_t i = 0; i < soundBlock.maxChannel; ++i)
	{
		const float* samples = soundBlock.samples[i];
		for (int32_t j = soundBlock.samplesCount - 1; j >= 0; --j)
		{
			if (abs(samples[j]) > c_isMuteThreshold)
			{
				outMuteOffset = j + 1;
				return false;
			}
		}
	}
	outMuteOffset = 0;
	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundPipeline", 32, SoundPipeline, editor::IPipeline)

SoundPipeline::SoundPipeline()
{
}

bool SoundPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
	const Guid& outputGuid
) const
{
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), soundAsset->getFileName().getOriginal());

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
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);

	Ref< IStreamDecoder > decoder;
	if (compareIgnoreCase< std::wstring >(soundAsset->getFileName().getExtension(), L"wav") == 0)
		decoder = new sound::WavStreamDecoder();
	else if (compareIgnoreCase< std::wstring >(soundAsset->getFileName().getExtension(), L"flac") == 0)
		decoder = new sound::FlacStreamDecoder();
	else if (compareIgnoreCase< std::wstring >(soundAsset->getFileName().getExtension(), L"mp3") == 0)
		decoder = new sound::Mp3StreamDecoder();
	else if (compareIgnoreCase< std::wstring >(soundAsset->getFileName().getExtension(), L"ogg") == 0)
		decoder = new sound::OggStreamDecoder();
	else
	{
		log::error << L"Failed to build sound asset, unable to determine decoder from extension" << Endl;
		return false;
	}

	Ref< IStream > sourceStream = pipelineBuilder->openFile(Path(m_assetPath), soundAsset->getFileName().getOriginal());
	if (!sourceStream)
	{
		log::error << L"Failed to build sound asset, unable to open source" << Endl;
		return false;
	}

	bool categorized = false;
	std::wstring configurationId;
	float gain = soundAsset->m_gain;
	float presence = soundAsset->m_presence;
	float presenceRate = soundAsset->m_presenceRate;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(soundAsset->m_category);

	if (category)
		configurationId = category->getConfigurationId();

	while (category)
	{
		categorized = true;
		gain += category->getGain();

		if (presence <= FUZZY_EPSILON)
		{
			presence = category->getPresence();
			presenceRate = category->getPresenceRate();
		}

		range = max(range, category->getRange());

		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	if (!categorized)
		log::warning << L"Uncategorized sound \"" << sourceInstance->getName() << L"\"" << Endl;

	log::info << L"Final gain " << gain << L" dB" << Endl;
	log::info << L"      presence " << presence << L", rate " << int32_t(presenceRate * 100.0f) << L" d%" << Endl;
	log::info << L"      range " << range << Endl;

	if (soundAsset->m_stream)
	{
		Ref< StreamSoundResource > resource = new StreamSoundResource();

		resource->m_decoderType = &type_of(decoder);
		resource->m_category = configurationId;
		resource->m_gain = gain;
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

		int64_t dataOffsetBegin = stream->tell();

		bool result = StreamCopy(stream, sourceStream).execute();

		int64_t dataOffsetEnd = stream->tell();

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

		int64_t dataOffsetBegin = stream->tell();

		// Prepare decoder with source stream.
		if (!decoder->create(sourceStream))
		{
			log::error << L"Failed to build sound asset, unable to create stream decoder" << Endl;
			instance->revert();
			return false;
		}

		// Prepare encoder with destination stream.
		Ref< IStreamEncoder > encoder = new OggStreamEncoder();
		if (!encoder->create(stream))
		{
			log::error << L"Failed to build sound asset, unable to create stream encoder" << Endl;
			instance->revert();
			return false;
		}

		// Transcode source stream.
		uint32_t sampleRate = 0;
		uint32_t samplesCount = 0;
		uint32_t maxChannel = 0;
		uint32_t mutedSamples = 0;

		SoundBlock soundBlock;
		std::memset(&soundBlock, 0, sizeof(soundBlock));
		soundBlock.samplesCount = 4096;

		while (decoder->getBlock(soundBlock))
		{
			if (
				soundBlock.samplesCount > 0 &&
				soundBlock.maxChannel > 0
			)
			{
				uint32_t muteOffset;
				if (!isMute(soundBlock, muteOffset))
				{
					T_ASSERT (muteOffset > 0);

					if (mutedSamples > 0)
					{
						SoundBlock muteBlock;
						std::memset(&muteBlock, 0, sizeof(muteBlock));
						muteBlock.samplesCount = mutedSamples;
						muteBlock.sampleRate = soundBlock.sampleRate;
						muteBlock.maxChannel = soundBlock.maxChannel;

						if (!encoder->putBlock(muteBlock))
						{
							log::error << L"Failed to build sound asset, transcoding failed" << Endl;
							return false;
						}

						mutedSamples = 0;
					}

					mutedSamples += soundBlock.samplesCount - muteOffset;
					soundBlock.samplesCount = muteOffset;

					if (!encoder->putBlock(soundBlock))
					{
						log::error << L"Failed to build sound asset, transcoding failed" << Endl;
						return false;
					}
				}
				else
				{
					mutedSamples += soundBlock.samplesCount;
				}
			}

			sampleRate = soundBlock.sampleRate;
			samplesCount += soundBlock.samplesCount;
			maxChannel = std::max(soundBlock.maxChannel, maxChannel);

			std::memset(&soundBlock, 0, sizeof(soundBlock));
			soundBlock.samplesCount = 4096;
		}

		log::info << L"Discarded " << mutedSamples << L" mute post sample(s)" << Endl;

		safeDestroy(encoder);
		safeDestroy(decoder);

		sourceStream->close();

		resource->m_category = configurationId;
		resource->m_sampleRate = sampleRate;
		resource->m_samplesCount = samplesCount;
		resource->m_channelsCount = maxChannel;
		resource->m_gain = gain;
		resource->m_presence = presence;
		resource->m_presenceRate = presenceRate;
		resource->m_range = range;
		resource->m_decoderType = &type_of< OggStreamDecoder >();

		int64_t dataOffsetEnd = stream->tell();
		stream->close();

		instance->setObject(resource);

		if (!instance->commit())
		{
			log::error << L"Failed to build sound asset, unable to commit instance" << Endl;
			return false;
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
