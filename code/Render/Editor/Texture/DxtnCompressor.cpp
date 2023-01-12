/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

#include <cstring>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/DxtnCompressor.h"

namespace traktor::render
{
	namespace
	{

struct CompressTextureTask
{
	const drawing::Image* image = nullptr;
	TextureFormat textureFormat = TfInvalid;
	bool needAlpha = false;
	int32_t compressionQuality = 0;
	AlignedVector< uint8_t > output;

	void execute()
	{
		const int32_t width = image->getWidth();
		const int32_t height = image->getHeight();

		const uint32_t outputSize = getTextureMipPitch(
			textureFormat,
			width,
			height
		);

		output.resize(outputSize, 0);

		const uint8_t* data = static_cast< const uint8_t* >(image->getData());
		uint8_t* block = &output[0];

		for (int32_t y = 0; y < height; y += 4)
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

						const uint32_t offset = (sx + sy * image->getWidth()) * 4;
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
				block += getTextureBlockSize(textureFormat);
			}
		}
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DxtnCompressor", DxtnCompressor, ICompressor)

bool DxtnCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	const int32_t mipCount = (int32_t)mipImages.size();
	for (int32_t i = 0; i < mipCount; ++i)
	{
		CompressTextureTask task;
		task.image = mipImages[i];
		task.textureFormat = textureFormat;
		task.needAlpha = needAlpha;
		task.compressionQuality = compressionQuality;
		task.execute();
		if (writer.write(task.output.c_ptr(), (int64_t)task.output.size(), 1) != task.output.size())
			return false;
	}
	return true;
}

}
