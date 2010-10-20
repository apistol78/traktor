#include <cstring>
#include "Compress/Zip/DeflateStream.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
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

int16_t quantify(float sample)
{
	sample = max(-1.0f, sample);
	sample = min( 1.0f, sample);
	return int16_t(sample * 32767.0f);
}

void analyzeDeltaRange(const int16_t* samples, uint32_t nsamples, int32_t& outNegD, int32_t& outPosD)
{
	outNegD = outPosD = 0;
	for (uint32_t i = 0; i < nsamples - 1; ++i)
	{
		int32_t delta = samples[i + 1] - samples[i];
		if (delta < 0)
			outNegD = max(outNegD, -delta);
		else
			outPosD = max(outPosD, delta);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundPipeline", 12, SoundPipeline, editor::IPipeline)

SoundPipeline::SoundPipeline()
:	m_sampleRate(44100)
,	m_enableZLibCompression(true)
,	m_bigEndian(false)
{
}

bool SoundPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_sampleRate = settings->getProperty< PropertyInteger >(L"SoundPipeline.SampleRate", m_sampleRate);
	m_enableZLibCompression = settings->getProperty< PropertyBoolean >(L"SoundPipeline.EnableZLibCompression", true);
	m_bigEndian = settings->getProperty< PropertyBoolean >(L"SoundPipeline.BigEndian", false);
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
		std::vector< int16_t > samples;

		SoundBlock soundBlock;
		std::memset(&soundBlock, 0, sizeof(soundBlock));
		soundBlock.samplesCount = 4096;

		float peek = 0.0f;
		while (decoder->getBlock(soundBlock))
		{
			uint32_t outputSamplesCount = (soundBlock.samplesCount * m_sampleRate) / soundBlock.sampleRate;

			for (uint32_t i = 0; i < outputSamplesCount; ++i)
			{
				for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
				{
					int16_t sample = 0;

					if (soundBlock.samples[j])
					{
						uint32_t p = (i * soundBlock.samplesCount) / outputSamplesCount;
						float s = soundBlock.samples[j][p];
						sample = quantify(s);
						peek = max(peek, s);
					}

					if (m_bigEndian)
						swap8in16(sample);

					samples.push_back(sample);
				}
			}

			samplesCount += outputSamplesCount;
			maxChannel = max(soundBlock.maxChannel, maxChannel);
		}

		if (peek > 1.0f)
			log::warning << L"Sound peek sample higher than 1; clipped to range" << Endl;

		sourceStream->close();

		resource->m_sampleRate = m_sampleRate;
		resource->m_samplesCount = samplesCount;
		resource->m_channelsCount = maxChannel;
		resource->m_flags = 0;

		if (samplesCount > 0)
		{
			uint32_t originalSize = samples.size() * sizeof(int16_t);

			// Compress ZLib
			Ref< DynamicMemoryStream > streamZLib = new DynamicMemoryStream(false, true);
			if (m_enableZLibCompression)
			{
				Ref< compress::DeflateStream > deflateZLib = new compress::DeflateStream(streamZLib);
				if (deflateZLib->write(&samples[0], samples.size() * sizeof(int16_t)) != samples.size() * sizeof(int16_t))
				{
					log::error << L"Failed to build sound asset, unable to write samples" << Endl;
					return false;
				}
				deflateZLib->close();
			}

			const std::vector< uint8_t >& zlibBuffer = streamZLib->getBuffer();
			if (m_enableZLibCompression && zlibBuffer.size() < originalSize)
			{
				log::info << L"Using ZLib, " << uint32_t(zlibBuffer.size()) << L" / " << originalSize << L" byte(s)" << Endl;

				resource->m_flags |= SrfZLib;
				
				if (stream->write(&zlibBuffer[0], zlibBuffer.size()) != zlibBuffer.size())
				{
					log::error << L"Failed to build sound asset, unable to write samples" << Endl;
					return false;
				}
			}
			else
			{
				log::info << L"Using no compression, " << originalSize << L" byte(s)" << Endl;

				if (stream->write(&samples[0], samples.size() * sizeof(int16_t)) != samples.size() * sizeof(int16_t))
				{
					log::error << L"Failed to build sound asset, unable to write samples" << Endl;
					return false;
				}
			}
		}

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

	}
}
