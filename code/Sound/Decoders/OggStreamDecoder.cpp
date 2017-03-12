#define OGG_VORBIS_DECODER_REF 1
#define OGG_VORBIS_DECODER_STB 2
#define OGG_VORBIS_DECODER OGG_VORBIS_DECODER_STB

#include <algorithm>
#include <cstring>
#if OGG_VORBIS_DECODER == OGG_VORBIS_DECODER_REF
#	include <vorbis/codec.h>
#elif OGG_VORBIS_DECODER == OGG_VORBIS_DECODER_STB
#	define STB_VORBIS_HEADER_ONLY
#	include <stb_vorbis.c>
#endif
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Decoders/OggStreamDecoder.h"

namespace traktor
{
	namespace sound
	{

#if OGG_VORBIS_DECODER == OGG_VORBIS_DECODER_REF

class OggStreamDecoderImpl : public Object
{
public:
	bool create(IStream* stream)
	{
		m_stream = stream;

		m_decoded[0] = (float*)Alloc::acquireAlign(sizeof(float) * 65536 * 2, 16, T_FILE_LINE);
		m_decoded[1] = (float*)Alloc::acquireAlign(sizeof(float) * 65536 * 2, 16, T_FILE_LINE);

		ogg_sync_init(&m_oy);

		// Submit a block to libvorbis' Ogg layer.
		m_buffer = ogg_sync_buffer(&m_oy, 8192);
		int bytes = stream->read(m_buffer, 8192);
		ogg_sync_wrote(&m_oy, bytes);

		if (ogg_sync_pageout(&m_oy, &m_og) != 1)
		{
			log::error << L"Failed to create Ogg stream, not OGG bitstream" << Endl;
			return false;
		}

		// Get the serial number and set up the rest of decode.
		// serialno first; use it to set up a logical stream.
		ogg_stream_init(&m_os, ogg_page_serialno(&m_og));

		// extract the initial header from the first page and verify that the
		// Ogg bitstream is in fact Vorbis data.

		// I handle the initial header first instead of just having the code
		// read all three Vorbis headers at once because reading the initial
		// header is an easy way to identify a Vorbis bitstream and it's
		// useful to see that functionality seperated out.

		vorbis_info_init(&m_vi);
		vorbis_comment_init(&m_vc);
		if (ogg_stream_pagein(&m_os, &m_og) < 0)
		{
			// error; stream version mismatch perhaps.
			log::error << L"Failed to create Ogg stream, error reading first page" << Endl;
			return false;
		}

		if (ogg_stream_packetout(&m_os, &m_op) != 1)
		{ 
			// no page? must not be vorbis.
			log::error << L"Failed to create Ogg stream, error reading initial header packet" << Endl;
			return false;
		}

		if (vorbis_synthesis_headerin(&m_vi, &m_vc, &m_op) < 0)
		{ 
			// error case; not a vorbis header.
			log::error << L"Failed to create Ogg stream, this Ogg bitstream does not contain Vorbis audio data" << Endl;
			return false;
		}		

		// At this point, we're sure we're Vorbis.  We've set up the logical
		// (Ogg) bitstream decoder.  Get the comment and codebook headers and
		// set up the Vorbis decoder

		// The next two packets in order are the comment and codebook headers.
		// They're likely large and may span multiple pages.  Thus we read
		// and submit data until we get our two packets, watching that no
		// pages are missing.  If a page is missing, error out; losing a
		// header page is the only place where missing data is fatal.

		int i = 0;
		while (i < 2)
		{
			while (i < 2)
			{
				int result = ogg_sync_pageout(&m_oy, &m_og);
				if (result == 0)
					break; // Need more data
				// Don't complain about missing or corrupt data yet.  We'll
				// catch it at the packet output phase.
				if (result == 1)
				{
					ogg_stream_pagein(&m_os, &m_og); // we can ignore any errors here as they'll also become apparent at packet-out.
					while (i < 2)
					{
						result = ogg_stream_packetout(&m_os, &m_op);
						if (result == 0)
							break;
						if (result < 0)
						{
							// Uh oh; data at some point was corrupted or missing!
							// We can't tolerate that in a header.  Die.
							log::error << L"Failed to create Ogg stream, corrupt secondary header" << Endl;
							return false;
						}
						vorbis_synthesis_headerin(&m_vi, &m_vc, &m_op);
						i++;
					}
				}
			}

			// no harm in not checking before adding more.
			m_buffer = ogg_sync_buffer(&m_oy, 8192);
			bytes = m_stream->read(m_buffer, 8192);
			if (bytes <= 0 && i < 2)
			{
				log::error << L"Failed to create Ogg stream, end of stream before finding all Vorbis headers" << Endl;
				return false;
			}
			ogg_sync_wrote(&m_oy, bytes);
		}

		// OK, got and parsed all three headers. Initialize the Vorbis
		// packet->PCM decoder.
		vorbis_synthesis_init(&m_vd, &m_vi); // central decode state
		vorbis_block_init(&m_vd, &m_vb);     // local state for most of the decode
											 // so multiple block decodes can
											 // proceed in parallel.  We could init
											 // multiple vorbis_block structures
											 // for vd here.

		m_readPage = true;
		m_readPacket = true;
		return true;
	}

	void destroy()
	{
		ogg_stream_clear(&m_os);

		// ogg_page and ogg_packet structs always point to storage in
		// libvorbis.  They're never freed or manipulated directly

		vorbis_block_clear(&m_vb);
		vorbis_dsp_clear(&m_vd);
		vorbis_comment_clear(&m_vc);
		vorbis_info_clear(&m_vi);  // must be called last

		// OK, clean up the framer
		ogg_sync_clear(&m_oy);

		Alloc::freeAlign(m_decoded[0]);
		Alloc::freeAlign(m_decoded[1]);
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(SoundBlock& outSoundBlock)
	{
		uint32_t decodedCount = 0;

		while (decodedCount < outSoundBlock.samplesCount)
		{
			// Read page if necessary.
			if (m_readPage)
			{
				for (;;)
				{
					int result = ogg_sync_pageout(&m_oy, &m_og);
					if (result >= 1)
						break;
					else if (result == 0)
					{
						m_buffer = ogg_sync_buffer(&m_oy, 8192);

						int bytes = m_stream->read(m_buffer, 8192);
						if (bytes <= 0)
							return false;

						ogg_sync_wrote(&m_oy, bytes);
					}
					else
						log::warning << L"Ogg decoder; corrupt or missing data in stream" << Endl;
				}

				ogg_stream_pagein(&m_os, &m_og);
				m_readPage = false;
			}

			// Read packet.
			if (m_readPacket)
			{
				for (;;)
				{
					int result = ogg_stream_packetout(&m_os, &m_op);
					if (result >= 1)
						break;
					else if (result == 0)
					{
						m_readPage = true;
						break;
					}
				}
				if (m_readPage)
					continue;
				m_readPacket = false;
			}

			// Decode packet.
			if (vorbis_synthesis(&m_vb, &m_op) == 0)
				vorbis_synthesis_blockin(&m_vd, &m_vb);

			float** pcm;
			int samplesCount;
			while ((samplesCount = vorbis_synthesis_pcmout(&m_vd, &pcm)) > 0)
			{
				int consumeSamples = std::min(int(outSoundBlock.samplesCount - decodedCount), samplesCount);

				for (int i = 0; i < m_vi.channels; ++i)
					std::memcpy(&m_decoded[i][decodedCount], pcm[i], consumeSamples * sizeof(float));

				decodedCount += consumeSamples;

				vorbis_synthesis_read(&m_vd, consumeSamples);

				if (decodedCount >= outSoundBlock.samplesCount)
					break;
			}

			if (samplesCount <= 0)
				m_readPacket = true;
		}

		// Prepare output sound block.
		for (int i = 0; i < m_vi.channels; ++i)
			outSoundBlock.samples[i] = m_decoded[i];
		outSoundBlock.maxChannel = m_vi.channels;
		outSoundBlock.sampleRate = m_vi.rate + 1400;	// \hack Why do we need this adjustment?
		outSoundBlock.samplesCount = decodedCount;

		return true;
	}

private:
	Ref< IStream > m_stream;
	ogg_sync_state m_oy;
	ogg_stream_state m_os;
	ogg_page m_og;
	ogg_packet m_op;
	vorbis_info m_vi;
	vorbis_comment m_vc;
	vorbis_dsp_state m_vd;
	vorbis_block m_vb;
	char* m_buffer;
	float* m_decoded[2];
	bool m_readPage;
	bool m_readPacket;
};

#elif OGG_VORBIS_DECODER == OGG_VORBIS_DECODER_STB

const wchar_t* const getErrorDescription(int32_t error)
{
	switch (error)
	{
	case VORBIS__no_error:
		return L"No error";
	case VORBIS_need_more_data:
		return L"Need more data";
	case VORBIS_invalid_api_mixing:
		return L"Invalid API mixing";
	case VORBIS_outofmem:
		return L"Out of memory";
	case VORBIS_feature_not_supported:
		return L"Feature not supported";
	case VORBIS_too_many_channels:
		return L"Too many channels";
	case VORBIS_file_open_failure:
		return L"File open failure";
	case VORBIS_seek_without_length:
		return L"Seek without length";
	case VORBIS_unexpected_eof:
		return L"Unexpected EOF";
	case VORBIS_seek_invalid:
		return L"Seek invalid";
	case VORBIS_invalid_setup:
		return L"Invalid setup";
	case VORBIS_invalid_stream:
		return L"Invalid stream";
	case VORBIS_missing_capture_pattern:
		return L"Missing capture pattern";
	case VORBIS_invalid_stream_structure_version:
		return L"Invalid stream structure version";
	case VORBIS_continued_packet_flag_invalid:
		return L"Continued packet flag invalid";
	case VORBIS_incorrect_stream_serial_number:
		return L"Incorrect stream serial number";
	case VORBIS_invalid_first_page:
		return L"Invalid first page";
	case VORBIS_bad_packet_type:
		return L"Bad packet type";
	case VORBIS_cant_find_last_page:
		return L"Can't find last page";
	case VORBIS_seek_failed:
		return L"Seek failed";
	default:
		return L"Unknown";
	}
}

class OggStreamDecoderImpl : public Object
{
public:
	OggStreamDecoderImpl()
	:	m_vorbis(0)
	,	m_buffered(0)
	,	m_output(0)
	,	m_channels(0)
	,	m_queued(0)
	,	m_consume(0)
	{
	}

	bool create(IStream* stream)
	{
		m_stream = stream;

		m_data.reset((uint8_t*)Alloc::acquireAlign(BufferSize, 16, T_FILE_LINE));
		m_decoded[0].reset((float*)Alloc::acquireAlign(DecodedSize, 16, T_FILE_LINE));
		m_decoded[1].reset((float*)Alloc::acquireAlign(DecodedSize, 16, T_FILE_LINE));
		m_buffered = 0;

		for (;;)
		{
			if (!read())
			{
				log::error << L"Unrecoverable OGG decoder error (Read data failed)" << Endl;
				return false;
			}

			int32_t used = 0, error = 0;
			m_vorbis = stb_vorbis_open_pushdata(m_data.ptr(), m_buffered, &used, &error, 0);
			if (m_vorbis)
				break;
			if (error != VORBIS_need_more_data)
			{
				log::error << L"Unrecoverable OGG decoder error (" << getErrorDescription(error) << L")" << Endl;
				return false;
			}

			if (used > 0)
				consume(used);
		}

		m_info = stb_vorbis_get_info(m_vorbis);
		return true;
	}

	void destroy()
	{
		stb_vorbis_close(m_vorbis);
		m_stream = 0;
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(SoundBlock& outSoundBlock)
	{
		m_queued -= m_consume;
		if (m_queued > 0)
		{
			for (int32_t i = 0; i < m_channels; ++i)
				std::memmove(m_decoded[i].ptr(), m_decoded[i].ptr() + m_consume, m_queued * sizeof(float));
		}
		m_consume = 0;

		while (m_queued < outSoundBlock.samplesCount)
		{
			int32_t decodedSamples = 0;
			while (decodedSamples <= 0)
			{
				int32_t used = stb_vorbis_decode_frame_pushdata(m_vorbis, m_data.ptr(), m_buffered, &m_channels, &m_output, &decodedSamples);
				if (used == 0)
				{
					if (read())
						continue;
					else
						return false;
				}

				T_ASSERT (used > 0)
				consume(used);
			}

			for (int32_t i = 0; i < m_channels; ++i)
				std::memcpy(m_decoded[i].ptr() + m_queued, m_output[i], decodedSamples * sizeof(float));

			m_queued += decodedSamples;
		}

		for (int32_t i = 0; i < m_channels; ++i)
			outSoundBlock.samples[i] = m_decoded[i].ptr();

		outSoundBlock.maxChannel = m_channels;
		outSoundBlock.sampleRate = m_info.sample_rate;

		m_consume = outSoundBlock.samplesCount;
		return true;
	}

private:
	enum
	{
		DecodedSize = 8192 * sizeof(float),
		BufferSize = 8 * 65536,
		PageSize = 4096
	};

	Ref< IStream > m_stream;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_data;
	AutoArrayPtr< float, AllocFreeAlign > m_decoded[2];
	stb_vorbis* m_vorbis;
	stb_vorbis_info m_info;
	int32_t m_buffered;
	float** m_output;
	int32_t m_channels;
	int32_t m_queued;
	int32_t m_consume;

	bool read()
	{
		int32_t nread = std::min< int32_t >(PageSize, BufferSize - m_buffered);
		T_ASSERT (nread > 0);

		int64_t read = m_stream->read(m_data.ptr() + m_buffered, nread);
		if (read > 0)
		{
			m_buffered += int32_t(read);
			return true;
		}
		else
			return false;
	}

	void consume(int32_t nbytes)
	{
		T_ASSERT (nbytes > 0);
		if (nbytes < m_buffered)
		{
			std::memmove(m_data.ptr(), m_data.ptr() + nbytes, m_buffered - nbytes);
			m_buffered -= nbytes;
		}
		else
			m_buffered = 0;
	}
};

#endif

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.OggStreamDecoder", 0, OggStreamDecoder, IStreamDecoder)

bool OggStreamDecoder::create(IStream* stream)
{
	m_stream = stream;
	rewind();
	return m_decoderImpl != 0;
}

void OggStreamDecoder::destroy()
{
	if (m_decoderImpl)
	{
		m_decoderImpl->destroy();
		m_decoderImpl = 0;
	}
}

double OggStreamDecoder::getDuration() const
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getDuration();
}

bool OggStreamDecoder::getBlock(SoundBlock& outSoundBlock)
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getBlock(outSoundBlock);
}

void OggStreamDecoder::rewind()
{
	destroy();
	m_stream->seek(IStream::SeekSet, 0);
	m_decoderImpl = new OggStreamDecoderImpl();
	if (!m_decoderImpl->create(m_stream))
		m_decoderImpl = 0;
}

	}
}
