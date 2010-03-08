#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Sound/StaticSoundResource.h"
#include "Sound/StreamSoundResource.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundPipeline.h"
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

inline int16_t quantify(float sample)
{
	sample = max(-1.0f, sample);
	sample = min( 1.0f, sample);
	return int16_t(sample * 32767.0f);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundPipeline", 2, SoundPipeline, editor::IPipeline)

SoundPipeline::SoundPipeline()
:	m_sampleRate(44100)
{
}

bool SoundPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	m_sampleRate = settings->getProperty< editor::PropertyInteger >(L"SoundPipeline.SampleRate", m_sampleRate);
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
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, soundAsset->getFileName());
	pipelineDepends->addDependency(fileName);
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
	if (compareIgnoreCase(fileName.getExtension(), L"wav") == 0)
		decoder = new sound::WavStreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"flac") == 0)
		decoder = new sound::FlacStreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"mp3") == 0)
		decoder = new sound::Mp3StreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"ogg") == 0)
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

	if (soundAsset->m_stream)
	{
		Ref< StreamSoundResource > resource = new StreamSoundResource(&type_of(decoder));

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

		bool result = StreamCopy(stream, sourceStream).execute();

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

		instance->setObject(resource);

		Ref< IStream > stream = instance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Failed to build sound asset, unable to create data stream" << Endl;
			instance->revert();
			return false;
		}

		// Prepare decoder with source stream.
		if (!decoder->create(sourceStream))
		{
			log::error << L"Failed to build sound asset, unable to create stream decoder" << Endl;
			return false;
		}

		// Decode source stream.
		uint32_t samplesCount = 0;
		uint32_t maxChannel = 0;
		std::vector< int16_t > samples[SbcMaxChannelCount];

		SoundBlock soundBlock;
		std::memset(&soundBlock, 0, sizeof(soundBlock));
		soundBlock.samplesCount = 4096;

		float peek = 0.0f;
		while (decoder->getBlock(soundBlock))
		{
			uint32_t outputSamplesCount = uint32_t(double(soundBlock.samplesCount * m_sampleRate) / soundBlock.sampleRate + 0.5);

			for (uint32_t i = 0; i < soundBlock.maxChannel; ++i)
			{
				for (uint32_t j = 0; j < outputSamplesCount; ++j)
				{
					uint32_t p0 = ((j + 0) * soundBlock.sampleRate) / m_sampleRate;
					uint32_t p1 = ((j + 1) * soundBlock.sampleRate) / m_sampleRate;

					float s0 = soundBlock.samples[i][p0];
					float s1 = soundBlock.samples[i][p1];
					float s = (s0 + s1) * 0.5f;

					samples[i].push_back(quantify(s));

					peek = max(peek, s);
				}
			}

			samplesCount += outputSamplesCount;
			maxChannel = max(soundBlock.maxChannel, maxChannel);
		}

		if (peek > 1.0f)
			log::warning << L"Sound peek sample higher than 1; clipped to range" << Endl;

		// Write asset.
		Writer writer(stream);
		writer << uint32_t(2);
		writer << uint32_t(m_sampleRate);
		writer << uint32_t(samplesCount);
		writer << uint32_t(maxChannel);

		for (uint32_t i = 0; i < maxChannel; ++i)
			writer.write(&samples[i][0], int(samples[i].size()), sizeof(int16_t));

		stream->close();
		sourceStream->close();

		if (!instance->commit())
		{
			log::error << L"Failed to build sound asset, unable to commit instance" << Endl;
			return false;
		}
	}

	return true;
}

	}
}
