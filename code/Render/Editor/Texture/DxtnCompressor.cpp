/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define SQUISH_COMPRESSOR	1
#define STB_DXT_COMPRESSOR	2
#define USE_DXT_COMPRESSOR	SQUISH_COMPRESSOR //STB_DXT_COMPRESSOR

#if USE_DXT_COMPRESSOR == SQUISH_COMPRESSOR
#	include <squish.h>
#elif USE_DXT_COMPRESSOR == STB_DXT_COMPRESSOR
#	define STB_DXT_IMPLEMENTATION
#	include <stb_dxt.h>
#endif

#include <algorithm>
#include <cstring>
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/DxtnCompressor.h"

namespace traktor::render
{
	namespace
	{

/*! Compress block rows [y0, y1) of an image, y0 must be a multiple of 4. */
void compressBlockRows(const drawing::Image* image, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality, int32_t y0, int32_t y1, uint8_t* block)
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();
	const uint32_t blockSize = getTextureBlockSize(textureFormat);
	const uint8_t* data = static_cast< const uint8_t* >(image->getData());

	for (int32_t y = y0; y < y1; y += 4)
	{
		for (int32_t x = 0; x < width; x += 4)
		{
			uint8_t rgba[4][4][4];
			int32_t mask = 0;

			std::memset(rgba, 0, sizeof(rgba));

			for (int iy = 0; iy < 4; ++iy)
			{
				for (int ix = 0; ix < 4; ++ix)
				{
					const int32_t sx = x + ix;
					const int32_t sy = y + iy;

					if (sx >= width || sy >= height)
						continue;

					const uint32_t offset = (sx + sy * width) * 4;
					rgba[iy][ix][0] = data[offset + 0];
					rgba[iy][ix][1] = data[offset + 1];
					rgba[iy][ix][2] = data[offset + 2];
					rgba[iy][ix][3] = needAlpha ? data[offset + 3] : 0xff;

					mask |= 1 << (ix + iy * 4);
				}
			}

#if USE_DXT_COMPRESSOR == SQUISH_COMPRESSOR
			const int32_t c_compressionFlags[] = { squish::kColourRangeFit, squish::kColourClusterFit, squish::kColourIterativeClusterFit };

			int32_t flags = c_compressionFlags[compressionQuality];
			if (textureFormat == TfDXT1)
				flags |= squish::kDxt1;
			else if (textureFormat == TfDXT3)
				flags |= squish::kDxt3;
			else if (textureFormat == TfDXT5)
				flags |= squish::kDxt5;

			if (needAlpha)
				flags |= squish::kWeightColourByAlpha;

			squish::CompressMasked(
				(const squish::u8*)rgba,
				mask,
				block,
				flags
			);
#elif USE_DXT_COMPRESSOR == STB_DXT_COMPRESSOR
			if (textureFormat == TfDXT1 || textureFormat == TfDXT5)
			{
				stb_compress_dxt_block(
					block,
					(const unsigned char*)rgba,
					needAlpha,
					compressionQuality > 0 ? STB_DXT_HIGHQUAL : STB_DXT_NORMAL
				);
			}
			else if (textureFormat == TfDXT3)
			{
				// Manually compress alpha as stb_dxt doesn't support DXT3.
				block[0] = (rgba[0][1][3] & 0xf0) | (rgba[0][0][3] >> 4);
				block[1] = (rgba[0][3][3] & 0xf0) | (rgba[0][2][3] >> 4);
				block[2] = (rgba[1][1][3] & 0xf0) | (rgba[1][0][3] >> 4);
				block[3] = (rgba[1][3][3] & 0xf0) | (rgba[1][2][3] >> 4);
				block[4] = (rgba[2][1][3] & 0xf0) | (rgba[2][0][3] >> 4);
				block[5] = (rgba[2][3][3] & 0xf0) | (rgba[2][2][3] >> 4);
				block[6] = (rgba[3][1][3] & 0xf0) | (rgba[3][0][3] >> 4);
				block[7] = (rgba[3][3][3] & 0xf0) | (rgba[3][2][3] >> 4);

				stb_compress_dxt_block(
					&block[8],
					(const unsigned char*)rgba,
					0,
					compressionQuality > 0 ? STB_DXT_HIGHQUAL : STB_DXT_NORMAL
				);
			}
#endif
			block += blockSize;
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DxtnCompressor", DxtnCompressor, ICompressor)

bool DxtnCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	const int32_t mipCount = (int32_t)mipImages.size();
	const uint32_t blockSize = getTextureBlockSize(textureFormat);

	// Allocate output of all mips up front and count total number of blocks.
	AlignedVector< AlignedVector< uint8_t > > outputs(mipCount);
	uint32_t totalBlockCount = 0;
	for (int32_t i = 0; i < mipCount; ++i)
	{
		const drawing::Image* image = mipImages[i];
		const uint32_t outputSize = getTextureMipPitch(textureFormat, image->getWidth(), image->getHeight());
		outputs[i].resize(outputSize, 0);
		totalBlockCount += outputSize / blockSize;
	}

	// Split work into slices of block rows, of roughly equal block count, across all mips.
	// Create a few slices per worker so varying per-block cost is balanced out as well.
	const uint32_t targetSliceCount = (JobManager::getInstance().getWorkerCount() + 1) * 4;
	const uint32_t blocksPerSlice = std::max< uint32_t >((totalBlockCount + targetSliceCount - 1) / targetSliceCount, 256);

	AlignedVector< Job::task_t > tasks;
	for (int32_t i = 0; i < mipCount; ++i)
	{
		const drawing::Image* image = mipImages[i];
		const int32_t height = image->getHeight();
		const uint32_t rowPitch = getTextureRowPitch(textureFormat, image->getWidth());
		const int32_t rowsPerSlice = std::max< int32_t >(blocksPerSlice / (rowPitch / blockSize), 1) * 4;

		for (int32_t y0 = 0; y0 < height; y0 += rowsPerSlice)
		{
			const int32_t y1 = std::min(y0 + rowsPerSlice, height);
			uint8_t* block = outputs[i].ptr() + (y0 / 4) * rowPitch;
			tasks.push_back([=]() {
				compressBlockRows(image, textureFormat, needAlpha, compressionQuality, y0, y1, block);
			});
		}
	}

	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

	for (const auto& output : outputs)
	{
		if (writer.write(output.c_ptr(), (int64_t)output.size(), 1) != output.size())
			return false;
	}

	return true;
}

}
