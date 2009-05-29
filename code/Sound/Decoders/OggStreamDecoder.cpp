#include <vorbis/codec.h>
#include "Sound/Decoders/OggStreamDecoder.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{

class OggStreamDecoderImpl : public Object
{
public:
	bool create(Stream* stream)
	{
		m_stream = stream;

		ogg_sync_init(&m_oy);

		// Submit a 4k block to libvorbis' Ogg layer.
		m_buffer = ogg_sync_buffer(&m_oy, 4096);
		int bytes = stream->read(m_buffer, 4096);
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
			while (i<2)
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
			m_buffer = ogg_sync_buffer(&m_oy, 4096);
			bytes = m_stream->read(m_buffer, 4096);
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
					if (result > 0)
						break;

					m_buffer = ogg_sync_buffer(&m_oy, 4096);
					int bytes = m_stream->read(m_buffer, 4096);
					ogg_sync_wrote(&m_oy, bytes);
					if (bytes <= 0)
						return false;
				}
				ogg_stream_pagein(&m_os, &m_og);
				m_readPage = false;
			}

			// Read packet.
			if (m_readPacket)
			{
				int result = ogg_stream_packetout(&m_os, &m_op);
				if (result <= 0)
				{
					m_readPage = true;
					continue;
				}
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
					memcpy(&m_decoded[i][decodedCount], pcm[i], consumeSamples * sizeof(float));

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
		outSoundBlock.channels = m_vi.channels;
		outSoundBlock.sampleRate = m_vi.rate;
		outSoundBlock.samplesCount = decodedCount;

		return true;
	}

private:
	Ref< Stream > m_stream;
	ogg_sync_state m_oy;
	ogg_stream_state m_os;
	ogg_page m_og;
	ogg_packet m_op;
	vorbis_info m_vi;
	vorbis_comment m_vc;
	vorbis_dsp_state m_vd;
	vorbis_block m_vb;
	char* m_buffer;
	float m_decoded[2][65535];
	bool m_readPage;
	bool m_readPacket;
};

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.OggStreamDecoder", OggStreamDecoder, StreamDecoder)

bool OggStreamDecoder::create(Stream* stream)
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
	m_stream->seek(Stream::SeekSet, 0);
	m_decoderImpl = gc_new< OggStreamDecoderImpl >();
	if (!m_decoderImpl->create(m_stream))
		m_decoderImpl = 0;
}

	}
}
