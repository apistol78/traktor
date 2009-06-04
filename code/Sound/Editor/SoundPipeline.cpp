#include "Sound/Editor/SoundPipeline.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/StaticSoundResource.h"
#include "Sound/StreamSoundResource.h"
#include "Sound/StreamDecoder.h"
#include "Sound/Decoders/WavStreamDecoder.h"
#include "Sound/Decoders/FlacStreamDecoder.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"
#include "Sound/Decoders/OggStreamDecoder.h"
#include "Editor/PipelineManager.h"
#include "Database/Instance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Writer.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.SoundPipeline", SoundPipeline, editor::Pipeline)

bool SoundPipeline::create(const editor::Settings* settings)
{
	return true;
}

void SoundPipeline::destroy()
{
}

uint32_t SoundPipeline::getVersion() const
{
	return 1;
}

TypeSet SoundPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SoundAsset >());
	return typeSet;
}

bool SoundPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool SoundPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const SoundAsset > soundAsset = checked_type_cast< const SoundAsset* >(sourceAsset);
	Path fileName = soundAsset->getFileName();

	Ref< StreamDecoder > decoder;
	if (compareIgnoreCase(fileName.getExtension(), L"wav") == 0)
		decoder = gc_new< sound::WavStreamDecoder >();
	else if (compareIgnoreCase(fileName.getExtension(), L"flac") == 0)
		decoder = gc_new< sound::FlacStreamDecoder >();
	else if (compareIgnoreCase(fileName.getExtension(), L"mp3") == 0)
		decoder = gc_new< sound::Mp3StreamDecoder >();
	else if (compareIgnoreCase(fileName.getExtension(), L"ogg") == 0)
		decoder = gc_new< sound::OggStreamDecoder >();
	else
	{
		log::error << L"Failed to build sound asset, unable to determine decoder from extension" << Endl;
		return false;
	}

	Ref< Stream > sourceStream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to build sound asset, unable to open source" << Endl;
		return false;
	}

	if (soundAsset->m_stream)
	{
		Ref< StreamSoundResource > resource = gc_new< StreamSoundResource >(&decoder->getType());

		Ref< db::Instance > instance = pipelineManager->createOutputInstance(
			outputPath,
			outputGuid,
			resource
		);
		if (!instance)
		{
			log::error << L"Failed to build sound asset, unable to create instance" << Endl;
			return false;
		}

		Ref< Stream > stream = instance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Failed to build sound asset, unable to create data stream" << Endl;
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
		Ref< StaticSoundResource > resource = gc_new< StaticSoundResource >();

		Ref< db::Instance > instance = pipelineManager->createOutputInstance(
			outputPath,
			outputGuid,
			resource
		);
		if (!instance)
		{
			log::error << L"Failed to build sound asset, unable to create instance" << Endl;
			return false;
		}

		Ref< Stream > stream = instance->writeData(L"Data");
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
		uint32_t sampleRate = 0;
		uint32_t samplesCount = 0;
		uint32_t channelsCount = 0;
		std::vector< int16_t > samples[SbcMaxChannelCount];

		SoundBlock soundBlock;
		memset(&soundBlock, 0, sizeof(soundBlock));
		soundBlock.samplesCount = 4096;

		while (decoder->getBlock(soundBlock))
		{
			for (uint32_t i = 0; i < soundBlock.maxChannel; ++i)
			{
				for (uint32_t j = 0; j < soundBlock.samplesCount; ++j)
					samples[i].push_back(quantify(soundBlock.samples[i][j]));
			}

			sampleRate = soundBlock.sampleRate;
			samplesCount += soundBlock.samplesCount;
			channelsCount = soundBlock.maxChannel;
		}

		// Write asset.
		Writer writer(stream);
		writer << uint32_t(2);
		writer << uint32_t(sampleRate);
		writer << uint32_t(samplesCount);
		writer << uint32_t(channelsCount);

		for (uint32_t i = 0; i < channelsCount; ++i)
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
