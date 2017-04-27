/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <theora/theoradec.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Video/Decoders/VideoDecoderTheora.h"

namespace traktor
{
	namespace video
	{
		namespace
		{
		
uint32_t YCbCr2RGB8(uint8_t Y, uint8_t Cb, uint8_t Cr)
{
	float r = Y * 298.082f / 256.0f + Cr * 408.583f / 256.0f - 222.921f;
	float g = Y * 298.082f / 256.0f - Cb * 100.291f / 256.0f - Cr * 208.120f / 256.0f + 135.576f;
	float b = Y * 298.082f / 256.0f + Cb * 516.412f / 256.0f - 276.836f;

	return
		(uint32_t(clamp(r, 0.0f, 255.0f))) |
		(uint32_t(clamp(g, 0.0f, 255.0f)) << 8) |
		(uint32_t(clamp(b, 0.0f, 255.0f)) << 16) |
		0xff000000;
}

		}

class VideoDecoderTheoraImpl : public Object
{
public:
	VideoDecoderTheoraImpl()
	:	m_ts(0)
	,	m_td(0)
	,	m_stateflag(0)
	,	m_theora_p(0)
	{
	}

	bool create(IStream* stream)
	{
		m_stream = stream;

		ogg_sync_init(&m_oy);

		// Init supporting Theora structures needed in header parsing.
		th_comment_init(&m_tc);
		th_info_init(&m_ti);

		// Ogg file open; parse the headers.
		// Theora (like Vorbis) depends on some initial header packets for decoder
		// setup and initialization.
		// We retrieve these first before entering the main decode loop.

		// Only interested in Theora streams.
		while (!m_stateflag)
		{
			int32_t ret = bufferData();
			if (ret <= 0)
				break;

			while (ogg_sync_pageout(&m_oy, &m_og) > 0)
			{
				ogg_stream_state test;

				// Is this a mandated initial header? If not, stop parsing.
				if (!ogg_page_bos(&m_og))
				{
					// Don't leak the page; get it into the appropriate stream.
					ogg_stream_pagein(&m_to, &m_og);
					m_stateflag = 1;
					break;
				}

				ogg_stream_init(&test, ogg_page_serialno(&m_og));
				ogg_stream_pagein(&test, &m_og);
				ogg_stream_packetout(&test, &m_op);

				// Identify the codec: try theora.
				if (!m_theora_p && th_decode_headerin(&m_ti, &m_tc, &m_ts, &m_op) >= 0)
				{
					std::memcpy(&m_to, &test, sizeof(test));
					m_theora_p=1;
				}
				else
				{
					// Whatever it is, we don't care about it.
					ogg_stream_clear(&test);
				}
			}

			// Fall through to non-bos page parsing.
		}

		// We're expecting more header packets.
		while (m_theora_p && m_theora_p < 3)
		{
			int ret;

			// Look for further theora headers.
			while(
				m_theora_p &&
				m_theora_p < 3 &&
				(ret = ogg_stream_packetout(&m_to, &m_op))
			)
			{
				if (ret < 0)
					continue;

				if (!th_decode_headerin(&m_ti, &m_tc, &m_ts, &m_op))
				{
					log::error << L"Error parsing Theora stream headers; corrupt stream?" << Endl;
					return false;
				}

				m_theora_p++;
			}

			// The header pages/packets will arrive before anything else we
			// care about, or the stream is not obeying spec.

			if (ogg_sync_pageout(&m_oy, &m_og) > 0)
			{
				// Demux into the appropriate stream.
				ogg_stream_pagein(&m_to, &m_og);
			}
			else
			{
				// Someone needs more data.
				int32_t ret = bufferData();
				if (ret <= 0)
				{
					log::error << L"End of file while searching for codec headers." << Endl;
					return false;
				}
			}
		}

		if (!m_theora_p)
			return false;

		m_td = th_decode_alloc(&m_ti, m_ts);

		int pp_level_max;
		int pp_level;

		th_decode_ctl(
			m_td,
			TH_DECCTL_GET_PPLEVEL_MAX,
			&pp_level_max,
			sizeof(pp_level_max)
		);

		pp_level = pp_level_max;

		th_decode_ctl(
			m_td,
			TH_DECCTL_SET_PPLEVEL,
			&pp_level,
			sizeof(pp_level)
		);

		T_DEBUG(L"Theora decoder created, " << m_ti.pic_width << L"x" << m_ti.pic_height << L", " << float(m_ti.fps_numerator / m_ti.fps_denominator) << L" fps");

		return true;
	}

	void destroy()
	{
		th_decode_free(m_td);
		th_comment_clear(&m_tc);
		th_info_clear(&m_ti);
		th_setup_free(m_ts);

		ogg_stream_clear(&m_to);
		ogg_sync_clear(&m_oy);
	}

	bool getInformation(VideoDecoderInfo& outInfo) const
	{
		outInfo.width = m_ti.pic_width;
		outInfo.height = m_ti.pic_height;
		outInfo.rate = float(m_ti.fps_numerator / m_ti.fps_denominator);
		return true;
	}

	bool decode(uint32_t frame, void* bits, uint32_t pitch)
	{
		ogg_int64_t videoGranulePosition = -1;
		double videoTime = 0.0;
		double frameTargetTime = frame / double(m_ti.fps_numerator / m_ti.fps_denominator);
		bool haveFrame = false;

		for (;;)
		{
			if (ogg_stream_packetout(&m_to, &m_op) > 0)
			{
				// HACK: This should be set after a seek or a gap, but we might not have
				// a granulepos for the first packet (we only have them for the last
				// packet on a page), so we just set it as often as we get it.
				// To do this right, we should back-track from the last packet on the
				// page and compute the correct granulepos for the first packet after
				// a seek or a gap.
				if (m_op.granulepos >= 0)
				{
					th_decode_ctl(
						m_td,
						TH_DECCTL_SET_GRANPOS,
						&m_op.granulepos,
						sizeof(m_op.granulepos)
					);
				}

				if (th_decode_packetin(m_td, &m_op, &videoGranulePosition) == 0)
				{
					videoTime = th_granule_time(m_td, videoGranulePosition);
					if (videoTime >= frameTargetTime)
					{
						haveFrame = true;
						break;
					}
				}
			}
			else
			{
				if (bufferData() <= 0)
					break;
				while (ogg_sync_pageout(&m_oy, &m_og) > 0)
					ogg_stream_pagein(&m_to, &m_og);
			}
		}

		if (!haveFrame)
			return false;

		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(m_td, yuv);

		if (m_ti.pixel_fmt == TH_PF_420)
		{
			for (uint32_t y = 0; y < m_ti.pic_height; ++y)
			{
				const uint8_t* inY = yuv[0].data + yuv[0].stride * y;
				const uint8_t* inU = yuv[1].data + yuv[1].stride * (y >> 1);
				const uint8_t* inV = yuv[2].data + yuv[2].stride * (y >> 1);

				uint32_t* rgba = static_cast< uint32_t* >(bits) + ((pitch * y) >> 2);

				for (uint32_t x = 0; x < m_ti.pic_width; ++x)
				{
					uint8_t Y = inY[x];
					uint8_t Cb = inU[x >> 1];
					uint8_t Cr = inV[x >> 1];
					*rgba++ = YCbCr2RGB8(Y, Cb, Cr);
				}
			}
		}
		else if (m_ti.pixel_fmt == TH_PF_422)
		{
			for (uint32_t y = 0; y < m_ti.pic_height; ++y)
			{
				const uint8_t* inY = yuv[0].data + yuv[0].stride * y;
				const uint8_t* inU = yuv[1].data + yuv[1].stride * (y >> 1);
				const uint8_t* inV = yuv[2].data + yuv[2].stride * (y >> 1);

				uint32_t* rgba = static_cast< uint32_t* >(bits) + ((pitch * y) >> 2);

				for (uint32_t x = 0; x < m_ti.pic_width; ++x)
				{
					uint8_t Y = inY[x];
					uint8_t Cb = inU[x];
					uint8_t Cr = inV[x];
					*rgba++ = YCbCr2RGB8(Y, Cb, Cr);
				}
			}
		}
		else if (m_ti.pixel_fmt == TH_PF_444)
		{
			for (uint32_t y = 0; y < m_ti.pic_height; ++y)
			{
				const uint8_t* inY = yuv[0].data + yuv[0].stride * y;
				const uint8_t* inU = yuv[1].data + yuv[1].stride * y;
				const uint8_t* inV = yuv[2].data + yuv[2].stride * y;

				uint32_t* rgba = static_cast< uint32_t* >(bits) + ((pitch * y) >> 2);

				for (uint32_t x = 0; x < m_ti.pic_width; ++x)
				{
					uint8_t Y = inY[x];
					uint8_t Cb = inU[x];
					uint8_t Cr = inV[x];
					*rgba++ = YCbCr2RGB8(Y, Cb, Cr);
				}
			}
		}

		return true;
	}

private:
	Ref< IStream > m_stream;
	ogg_sync_state m_oy;
	ogg_stream_state m_to;
	ogg_page m_og;
	ogg_packet m_op;
	th_info m_ti;
	th_comment m_tc;
	th_setup_info* m_ts;
	th_dec_ctx* m_td;
	int m_stateflag;
	int m_theora_p;

	int32_t bufferData()
	{
		char* buffer = ogg_sync_buffer(&m_oy, 4096);
		int32_t bytes = m_stream->read(buffer, 4096);
		ogg_sync_wrote(&m_oy, bytes);
		return bytes;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoDecoderTheora", VideoDecoderTheora, IVideoDecoder)

bool VideoDecoderTheora::create(IStream* stream)
{
	m_stream = stream;

	m_impl = new VideoDecoderTheoraImpl();
	if (!m_impl->create(stream))
	{
		m_impl = 0;
		return false;
	}

	return true;
}

void VideoDecoderTheora::destroy()
{
	safeDestroy(m_impl);
}

bool VideoDecoderTheora::getInformation(VideoDecoderInfo& outInfo) const
{
	return m_impl ? m_impl->getInformation(outInfo) : false;
}

void VideoDecoderTheora::rewind()
{
	safeDestroy(m_impl);

	m_stream->seek(IStream::SeekSet, 0);

	m_impl = new VideoDecoderTheoraImpl();
	if (!m_impl->create(m_stream))
		m_impl = 0;
}

bool VideoDecoderTheora::decode(uint32_t frame, void* bits, uint32_t pitch)
{
	return m_impl ? m_impl->decode(frame, bits, pitch) : false;
}

	}
}
