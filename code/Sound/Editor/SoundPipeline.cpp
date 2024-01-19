/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Sound/StaticAudioResource.h"
#include "Sound/StreamAudioResource.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/SoundPipeline.h"
#include "Sound/Editor/Encoders/OggStreamEncoder.h"
#include "Sound/Editor/Encoders/TssStreamEncoder.h"
#include "Sound/Decoders/FlacStreamDecoder.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"
#include "Sound/Decoders/OggStreamDecoder.h"
#include "Sound/Decoders/TssStreamDecoder.h"
#include "Sound/Decoders/WavStreamDecoder.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const float c_isMuteThreshold = 16.0f / 32767.0f;

bool isMute(const AudioBlock& block, uint32_t& outMuteOffset)
{
	for (uint32_t i = 0; i < block.maxChannel; ++i)
	{
		const float* samples = block.samples[i];
		for (int32_t j = block.samplesCount - 1; j >= 0; --j)
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundPipeline", 33, SoundPipeline, editor::IPipeline)

SoundPipeline::SoundPipeline()
{
}

bool SoundPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void SoundPipeline::destroy()
{
}

TypeInfoSet SoundPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SoundAsset >();
}

bool SoundPipeline::shouldCache() const
{
	return true;
}

uint32_t SoundPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool SoundPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SoundAsset* soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
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
	const SoundAsset* soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	const std::wstring extension = soundAsset->getFileName().getExtension();

	Ref< IStreamDecoder > decoder;
	if (compareIgnoreCase(extension, L"wav") == 0)
		decoder = new sound::WavStreamDecoder();
	else if (compareIgnoreCase(extension, L"flac") == 0)
		decoder = new sound::FlacStreamDecoder();
	else if (compareIgnoreCase(extension, L"mp3") == 0)
		decoder = new sound::Mp3StreamDecoder();
	else if (compareIgnoreCase(extension, L"ogg") == 0)
		decoder = new sound::OggStreamDecoder();
	else if (compareIgnoreCase(extension, L"tss") == 0)
		decoder = new sound::TssStreamDecoder();
	else
	{
		log::error << L"Failed to build sound asset, unable to determine decoder from extension \"" << extension << L"\"." << Endl;
		return false;
	}

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + soundAsset->getFileName());
	Ref< IStream > sourceStream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to build sound asset, unable to open source \"" << filePath.getPathName() << L"\"." << Endl;
		return false;
	}

	bool categorized = false;
	std::wstring configurationId;
	float gain = soundAsset->m_gain;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(soundAsset->m_category);

	if (category)
		configurationId = category->getConfigurationId();

	while (category)
	{
		categorized = true;
		gain += category->getGain();
		range = max(range, category->getRange());
		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	if (!categorized)
		log::warning << L"Uncategorized sound \"" << sourceInstance->getName() << L"\"" << Endl;

	log::info << L"Final gain " << gain << L" dB" << Endl;
	log::info << L"      range " << range << Endl;

	if (soundAsset->m_stream)
	{
		Ref< StreamAudioResource > resource = new StreamAudioResource();

		resource->m_decoderType = &type_of(decoder);
		resource->m_category = configurationId;
		resource->m_gain = gain;
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
		Ref< StaticAudioResource > resource = new StaticAudioResource();

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
		Ref< IStreamEncoder > encoder;

		if (soundAsset->getCompressed())
			encoder = new OggStreamEncoder();
		else
			encoder = new TssStreamEncoder();

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

		AudioBlock block;
		std::memset(&block, 0, sizeof(block));
		block.samplesCount = 4096;

		while (decoder->getBlock(block))
		{
			if (
				block.samplesCount > 0 &&
				block.maxChannel > 0
			)
			{
				uint32_t muteOffset;
				if (!isMute(block, muteOffset))
				{
					T_ASSERT(muteOffset > 0);

					if (mutedSamples > 0)
					{
						AudioBlock muteBlock;
						std::memset(&muteBlock, 0, sizeof(muteBlock));
						muteBlock.samplesCount = mutedSamples;
						muteBlock.sampleRate = block.sampleRate;
						muteBlock.maxChannel = block.maxChannel;

						if (!encoder->putBlock(muteBlock))
						{
							log::error << L"Failed to build sound asset, transcoding failed" << Endl;
							return false;
						}

						mutedSamples = 0;
					}

					mutedSamples += block.samplesCount - muteOffset;
					block.samplesCount = muteOffset;

					if (!encoder->putBlock(block))
					{
						log::error << L"Failed to build sound asset, transcoding failed" << Endl;
						return false;
					}
				}
				else
				{
					mutedSamples += block.samplesCount;
				}
			}

			sampleRate = block.sampleRate;
			samplesCount += block.samplesCount;
			maxChannel = std::max(block.maxChannel, maxChannel);

			std::memset(&block, 0, sizeof(block));
			block.samplesCount = 4096;
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
		resource->m_range = range;

		if (soundAsset->getCompressed())
			resource->m_decoderType = &type_of< OggStreamDecoder >();
		else
			resource->m_decoderType = &type_of< TssStreamDecoder >();

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

Ref< ISerializable > SoundPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
