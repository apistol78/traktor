/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstdlib>
#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/Editor/Encoders/OggStreamEncoder.h"

namespace traktor
{
	namespace sound
	{

class OggStreamEncoderImpl : public Object
{
public:
	bool create(IStream* stream)
	{
		m_stream = stream;
		m_encoderInitialized = false;
		return true;
	}

	void destroy()
	{
		if (m_encoderInitialized)
		{
			// Write end-of-stream to make sure OGG/Vorbis stream is terminated properly.
			vorbis_analysis_wrote(&m_dspState, 0);
			encodeBlocks();

			// Close OGG/Vorbis encoder.
			ogg_stream_clear(&m_outputStream);
			vorbis_block_clear(&m_block);
			vorbis_dsp_clear(&m_dspState);
			vorbis_comment_clear(&m_comment);
			vorbis_info_clear(&m_info);
		}

		m_stream = 0;
		m_encoderInitialized = false;
	}

	bool putBlock(SoundBlock& block)
	{
		// Initialize encoder with first block as it contain information about sample rate etc.
		if (!m_encoderInitialized)
		{
			if (!initializeEncoder(block.sampleRate, block.maxChannel))
				return false;

			m_encoderInitialized = true;
		}

		// Submit block to encoder.
		float** analysisBuffer = vorbis_analysis_buffer(&m_dspState, block.samplesCount * block.maxChannel);
		if (!analysisBuffer)
			return false;

		for (uint32_t i = 0; i < block.maxChannel; ++i)
		{
			if (block.samples[i])
			{
				for (uint32_t j = 0; j < block.samplesCount; ++j)
					analysisBuffer[i][j] = block.samples[i][j];
			}
			else
			{
				for (uint32_t j = 0; j < block.samplesCount; ++j)
					analysisBuffer[i][j] = 0.0f;
			}
		}
		vorbis_analysis_wrote(&m_dspState, block.samplesCount);

		// Finally lets encode blocks.
		encodeBlocks();
		return true;
	}

private:
	Ref< IStream > m_stream;
	bool m_encoderInitialized;
	vorbis_info m_info;
	vorbis_comment m_comment;
	vorbis_dsp_state m_dspState;
	vorbis_block m_block;
	ogg_stream_state m_outputStream;
	ogg_page m_page;
	ogg_packet m_packet;

	bool initializeEncoder(uint32_t sampleRate, uint32_t channels)
	{
		int32_t ret;

		vorbis_info_init(&m_info);
		ret = vorbis_encode_init_vbr(&m_info, channels, sampleRate, 0.8f);
		if (ret)
		{
			log::error << L"Unable to initialize OGG/Vorbis encoder" << Endl;
			return false;
		}

		vorbis_comment_init(&m_comment);
		vorbis_comment_add_tag(&m_comment, "ENCODER", "Traktor");

		vorbis_analysis_init(&m_dspState, &m_info);
		vorbis_block_init(&m_dspState, &m_block);

		ogg_stream_init(&m_outputStream, std::rand());

		// Write stream header.
		{
			ogg_packet header;
			ogg_packet headerComments;
			ogg_packet headerCodeTable;

			vorbis_analysis_headerout(&m_dspState, &m_comment, &header, &headerComments, &headerCodeTable);
			ogg_stream_packetin(&m_outputStream, &header);
			ogg_stream_packetin(&m_outputStream, &headerComments);
			ogg_stream_packetin(&m_outputStream, &headerCodeTable);

			for (;;)
			{
				int result = ogg_stream_flush(&m_outputStream, &m_page);
				if(result == 0)
					break;

				m_stream->write(m_page.header, m_page.header_len);
				m_stream->write(m_page.body, m_page.body_len);
			}
		}

		return true;
	}

	void encodeBlocks()
	{
		while (vorbis_analysis_blockout(&m_dspState, &m_block) == 1)
		{
			// Analysis, assume we want to use bitrate management.
			vorbis_analysis(&m_block, 0);
			vorbis_bitrate_addblock(&m_block);

			while(vorbis_bitrate_flushpacket(&m_dspState, &m_packet))
			{
				// Weld the packet into the bitstream.
				ogg_stream_packetin(&m_outputStream, &m_packet);

				// Write out pages (if any).
				for (;;)
				{
					int32_t res = ogg_stream_pageout(&m_outputStream, &m_page);
					if (res == 0)
						break;

					m_stream->write(m_page.header, m_page.header_len);
					m_stream->write(m_page.body, m_page.body_len);
				}
			}
		}
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.OggStreamEncoder", OggStreamEncoder, IStreamEncoder)

bool OggStreamEncoder::create(IStream* stream)
{
	m_impl = new OggStreamEncoderImpl();
	if (!m_impl->create(stream))
	{
		m_impl = 0;
		return false;
	}
	return true;
}

void OggStreamEncoder::destroy()
{
	safeDestroy(m_impl);
}

bool OggStreamEncoder::putBlock(SoundBlock& block)
{
	return m_impl ? m_impl->putBlock(block) : false;
}

	}
}
