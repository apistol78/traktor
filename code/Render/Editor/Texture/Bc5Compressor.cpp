/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/Bc5Compressor.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Writer.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/MathUtils.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"

#include <cmath>

namespace traktor::render
{
	namespace
	{

/*! Number of endpoint refinements per compression quality. */
const int32_t c_refineIterations[] = { 0, 2, 8 };

/*! Map a linear step, where zero is the low endpoint, onto a BC4 index.
 *
 * The two extremes are encoded as index 0 and 1 respectively, the remaining
 * six interpolated values follow in descending order.
 */
const uint8_t c_stepToIndex[] = { 1, 7, 6, 5, 4, 3, 2, 0 };

/*! Quantize each value to the closest of the eight palette entries.
 *
 * \return Sum of squared error of the block.
 */
float selectSteps(const uint8_t* T_RESTRICT values, int32_t e0, int32_t e1, uint8_t* T_RESTRICT outSteps)
{
	const float base = (float)e1;
	const float range = (float)(e0 - e1);
	const float scale = 7.0f / range;

	float error = 0.0f;
	for (int32_t i = 0; i < 16; ++i)
	{
		const int32_t step = clamp((int32_t)(((float)values[i] - base) * scale + 0.5f), 0, 7);
		outSteps[i] = (uint8_t)step;

		const float d = (float)values[i] - (base + range * (float)step / 7.0f);
		error += d * d;
	}
	return error;
}

/*! Least squares fit of both endpoints given a fixed set of indices.
 *
 * \return True if the fitted endpoints are usable, i.e. still ordered so the
 *         block remains in the eight value interpolation mode.
 */
bool refitEndpoints(const uint8_t* T_RESTRICT values, const uint8_t* T_RESTRICT steps, int32_t& outE0, int32_t& outE1)
{
	float a = 0.0f, b = 0.0f, c = 0.0f, x = 0.0f, y = 0.0f;
	for (int32_t i = 0; i < 16; ++i)
	{
		const float t = (float)steps[i] / 7.0f;
		const float s = 1.0f - t;

		a += t * t;
		b += s * t;
		c += s * s;
		x += (float)values[i] * t;
		y += (float)values[i] * s;
	}

	// Singular when every texel resolved to the same index; nothing to refine.
	const float det = a * c - b * b;
	if (std::abs(det) <= 1e-6f)
		return false;

	outE0 = clamp((int32_t)((c * x - b * y) / det + 0.5f), 0, 255);
	outE1 = clamp((int32_t)((a * y - b * x) / det + 0.5f), 0, 255);

	return outE0 > outE1;
}

/*! Compress a single channel of a 4x4 block into eight bytes of BC4.
 *
 * Endpoints are initially fitted to the extremes of the block, which is exact
 * at both ends but leaves the interior slightly biased. Each refinement then
 * re-fits the endpoints against the indices they produced, keeping whichever
 * pair yields the lowest error.
 */
void compressBc4Block(uint8_t* T_RESTRICT dest, const uint8_t* T_RESTRICT values, int32_t refineIterations)
{
	int32_t mn = values[0];
	int32_t mx = values[0];
	for (int32_t i = 1; i < 16; ++i)
	{
		mn = min< int32_t >(mn, values[i]);
		mx = max< int32_t >(mx, values[i]);
	}

	// Constant block; all indices reference the first endpoint.
	if (mn == mx)
	{
		dest[0] = (uint8_t)mx;
		dest[1] = (uint8_t)mn;
		for (int32_t i = 2; i < 8; ++i)
			dest[i] = 0x00;
		return;
	}

	uint8_t steps[16];

	int32_t bestE0 = mx;
	int32_t bestE1 = mn;
	float bestError = selectSteps(values, bestE0, bestE1, steps);

	for (int32_t i = 0; i < refineIterations; ++i)
	{
		int32_t e0, e1;
		if (!refitEndpoints(values, steps, e0, e1))
			break;

		const float error = selectSteps(values, e0, e1, steps);
		if (error >= bestError)
			break;

		bestError = error;
		bestE0 = e0;
		bestE1 = e1;
	}

	// The loop may have left indices belonging to a rejected pair.
	selectSteps(values, bestE0, bestE1, steps);

	dest[0] = (uint8_t)bestE0;
	dest[1] = (uint8_t)bestE1;

	uint64_t indices = 0;
	for (int32_t i = 0; i < 16; ++i)
		indices |= (uint64_t)c_stepToIndex[steps[i]] << (i * 3);

	for (int32_t i = 0; i < 6; ++i)
		dest[2 + i] = (uint8_t)(indices >> (i * 8));
}

struct CompressTextureTask : public RefCountImpl< IRefCount >
{
	const drawing::Image* image = nullptr;
	TextureFormat textureFormat = TfInvalid;
	int32_t compressionQuality = 0;
	AlignedVector< uint8_t > output;

	void execute()
	{
		const int32_t width = image->getWidth();
		const int32_t height = image->getHeight();

		output.resize(getTextureMipPitch(textureFormat, width, height), 0);

		const int32_t refineIterations = c_refineIterations[clamp(compressionQuality, 0, (int32_t)sizeof_array(c_refineIterations) - 1)];
		uint8_t* block = &output[0];
		Color4f texel;

		for (int32_t y = 0; y < height; y += 4)
		{
			for (int32_t x = 0; x < width; x += 4)
			{
				uint8_t r[16];
				uint8_t g[16];

				for (int32_t iy = 0; iy < 4; ++iy)
				{
					// Blocks which extend beyond the image replicate the edge
					// texel; padding with zero would stretch the endpoints.
					const int32_t sy = min(y + iy, height - 1);
					for (int32_t ix = 0; ix < 4; ++ix)
					{
						const int32_t sx = min(x + ix, width - 1);
						image->getPixelUnsafe(sx, sy, texel);

						r[ix + iy * 4] = (uint8_t)clamp((int32_t)((float)texel.getRed() * 255.0f + 0.5f), 0, 255);
						g[ix + iy * 4] = (uint8_t)clamp((int32_t)((float)texel.getGreen() * 255.0f + 0.5f), 0, 255);
					}
				}

				compressBc4Block(block, r, refineIterations);
				compressBc4Block(block + 8, g, refineIterations);

				block += getTextureBlockSize(textureFormat);
			}
		}
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Bc5Compressor", Bc5Compressor, ICompressor)

bool Bc5Compressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	RefArray< CompressTextureTask > tasks;
	RefArray< Job > jobs;

	const int32_t mipCount = (int32_t)mipImages.size();
	for (int32_t i = 0; i < mipCount; ++i)
	{
		Ref< CompressTextureTask > task = new CompressTextureTask();
		task->image = mipImages[i];
		task->textureFormat = textureFormat;
		task->compressionQuality = compressionQuality;

		jobs.push_back(
			JobManager::getInstance().add([=]() { task->execute(); }));

		tasks.push_back(task);
	}

	for (size_t i = 0; i < jobs.size(); ++i)
	{
		jobs[i]->wait();
		jobs[i] = nullptr;
	}

	for (auto task : tasks)
	{
		if (writer.write(task->output.c_ptr(), (int64_t)task->output.size(), 1) != task->output.size())
			return false;
	}

	return true;
}

}
