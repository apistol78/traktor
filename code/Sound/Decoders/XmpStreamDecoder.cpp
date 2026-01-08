#pragma optimize("", off)

/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Decoders/XmpStreamDecoder.h"

#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"

#include <xmp.h>

#define SAMPLE_RATE 44100

namespace traktor::sound
{
namespace
{

void render_channel_pcm(
	const struct xmp_module* mod,
	const xmp_channel_info* info,
	float* channel_buffer,
	double* current_offset,
	int samples_to_render)
{
	if (info->period <= 0 || info->sample < 0 || info->volume == 0)
		return;

	if (info->sample >= mod->smp)
		return;

	const struct xmp_sample* smp = &mod->xxs[info->sample];
	const struct xmp_instrument* inst = &mod->xxi[info->instrument];

	if (!smp->data || smp->len <= 0)
		return;

#define C4_PERIOD 428.0
#define C4_NTSC_RATE 8363
	double step = C4_PERIOD * C4_NTSC_RATE / ((double)SAMPLE_RATE) / (info->period / 4096.0);

	const int16_t* sample_data_s16 = (const int16_t*)smp->data;
	const int8_t* sample_data_u8 = (const int8_t*)smp->data;

	const float v0 = (float)info->volume / 64.0f;
	const float v1 = (float)inst->vol / 64.0f;
	const float v2 = (float)info->pan / 256.0f;
	const float gain = v0 * v1;

	double pos = *current_offset;
	float* out = channel_buffer;

	for (int i = 0; i < samples_to_render; i++)
	{
		int pos_i = (int)pos;

		if (smp->flg & XMP_SAMPLE_LOOP)
		{
			if (pos_i >= smp->lpe)
			{
				pos = (double)smp->lps;
				pos_i = (int)pos;
			}
		}

		if (pos_i >= smp->len)
		{
			if (smp->flg & XMP_SAMPLE_LOOP_FULL)
			{
				pos = 0;
				pos_i = 0;
			}
			else
			{
				pos = smp->len;
				break;
			}
		}

		float sample1 = 0.0f;
		if (pos_i >= 0 && pos_i < smp->len)
		{
			if (smp->flg & XMP_SAMPLE_16BIT)
				sample1 = ((float)sample_data_s16[pos_i]) / 32768.0f;
			else
				sample1 = ((float)sample_data_u8[pos_i]) / 128.0f;
		}

		*out++ = sample1 * gain;
		pos += step;
	}

	*current_offset = pos;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.XmpStreamDecoder", 0, XmpStreamDecoder, IStreamDecoder)

XmpStreamDecoder::XmpStreamDecoder()
{
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		m_samplesBuffer[i] = nullptr;
}

bool XmpStreamDecoder::create(IStream* stream)
{
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		m_samplesBuffer[i] = (float*)Alloc::acquireAlign(4096 * sizeof(float), 16, T_FILE_LINE);
		if (!m_samplesBuffer[i])
			return false;
	}

	AlignedVector< uint8_t > md;
	DynamicMemoryStream mds(md, false, true);
	StreamCopy(&mds, stream).execute();

	m_ctx = xmp_create_context();
	xmp_load_module_from_memory((xmp_context)m_ctx, md.c_ptr(), md.size());
	xmp_start_player((xmp_context)m_ctx, 44100, 0);

	memset(channel_offsets, 0, sizeof(channel_offsets));

	return true;
}

void XmpStreamDecoder::destroy()
{
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		if (m_samplesBuffer[i])
		{
			Alloc::freeAlign(m_samplesBuffer[i]);
			m_samplesBuffer[i] = nullptr;
		}
	}

	xmp_free_context((xmp_context)m_ctx);
	m_ctx = nullptr;
}

double XmpStreamDecoder::getDuration() const
{
	return 0.0;
}

bool XmpStreamDecoder::getBlock(AudioBlock& outBlock)
{
	int16_t buffer[1024 * 2];
	if (xmp_play_buffer((xmp_context)m_ctx, buffer, 1024 * 2 * sizeof(int16_t), 0) != 0)
		return false;

	for (int32_t i = 0; i < 1024; ++i)
	{
		const int16_t ls = buffer[i * 2 + 0];
		const int16_t rs = buffer[i * 2 + 1];

		m_samplesBuffer[0][i] = (float)ls / 32767.0f;
		m_samplesBuffer[1][i] = (float)rs / 32767.0f;
	}

	outBlock.samplesCount = 1024;
	outBlock.sampleRate = 44100;
	outBlock.maxChannel = 2;
	outBlock.samples[0] = m_samplesBuffer[0];
	outBlock.samples[1] = m_samplesBuffer[1];

	/*
	xmp_module_info mi;
	xmp_get_module_info((xmp_context)m_ctx, &mi);
	const int numChannels = mi.mod->chn;

	xmp_play_frame((xmp_context)m_ctx);

	xmp_frame_info fi;
	xmp_get_frame_info((xmp_context)m_ctx, &fi);

	const int32_t samplesPerFrame = (int)(SAMPLE_RATE * fi.frame_time / 1000000.0);

	for (int i = 0; i < numChannels; ++i)
	{
		std::memset(m_samplesBuffer[i], 0, samplesPerFrame * sizeof(float));

		const xmp_channel_info* ci = &fi.channel_info[i];

		if (ci->period <= 0 || ci->sample < 0 || ci->volume == 0)
			continue;

		xmp_sample* sm = &mi.mod->xxs[ci->sample];

		if (ci->position >= 0 && ci->position != (int)channel_offsets[i])
			channel_offsets[i] = (double)ci->position;

		render_channel_pcm(mi.mod, ci, m_samplesBuffer[i], &channel_offsets[i], samplesPerFrame);
	}

	for (int j = 0; j < samplesPerFrame; ++j)
	{
		m_samplesBuffer[0][j] *= 0.1f;
		m_samplesBuffer[1][j] *= 0.1f;
	}

	for (int i = 2; i < numChannels; i += 2)
	{
		for (int j = 0; j < samplesPerFrame; ++j)
		{
			m_samplesBuffer[0][j] += m_samplesBuffer[i + 0][j] * 0.1f;
			m_samplesBuffer[1][j] += m_samplesBuffer[i + 1][j] * 0.1f;
		}
	}

	outBlock.samplesCount = samplesPerFrame;
	outBlock.sampleRate = 44100;
	outBlock.maxChannel = 2;
	outBlock.samples[0] = m_samplesBuffer[0];
	outBlock.samples[1] = m_samplesBuffer[1];
	*/
	return true;
}

void XmpStreamDecoder::rewind()
{
}

}
