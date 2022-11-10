/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/EncodeRGBM.h"

namespace traktor::drawing
{
	namespace
	{

// Force val[N+1] to be contrained to not be less than val[N]*(R/256).
void imposeRatio(uint8_t* data, int32_t length, int32_t stride, int32_t ratio)
{
    uint8_t val = *data;
    for (int32_t i = 1; i < length; i++)
	{
        data += stride;

        int32_t min_val = (val * ratio) >> 8;
        uint8_t vl2 = *data;
        if (vl2 < min_val)
            *data = vl2 = min_val;

        val = vl2;
    }
}

// https://github.com/ARM-software/astc-encoder/blob/master/Utils/astc_rgbm_blockify.cpp
void prepareForBlockCompression(drawing::Image* image, int32_t blockDimX, int32_t blockDimY, float ratio)
{
	const int32_t ratioI = (int32_t)std::floor(ratio * 256.0f + 0.5f);

	int32_t blockCountX = (image->getWidth() + blockDimX - 1) / blockDimX;
	int32_t blockCountY = (image->getHeight() + blockDimY - 1) / blockDimY;
	int32_t leftOverY = blockDimY * blockCountY - image->getHeight();

	// For each block, obtain the maximum M-value.
	AlignedVector< uint8_t > mbuf;
	mbuf.resize(blockCountX * blockCountY, 0);

	uint8_t* imageData = (uint8_t*)image->getData();
	for (int32_t y = 0; y < image->getHeight(); y++)
	{
		uint8_t* s = imageData + 4 * y * image->getWidth() + 3;
		uint8_t* d = mbuf.ptr() + ((y + leftOverY) / blockDimY) * blockCountX;

		int32_t ctr = 0;

		int32_t accum = *d;
		for (int x = 0; x < image->getWidth(); x++)
		{
			uint8_t p = s[4 * x];
			if (p > accum)
				accum = p;

			if (++ctr == blockDimX)
			{
				*d++ = accum;
				accum = *d;
				ctr = 0;
			}
		}

		if (ctr != 0) {
			*d = accum;
		}
	}

    // Impose ratio restriction on M-values in adjacent blocks.
	for (int32_t y = 0;  y < blockCountY; y++)
	{
		imposeRatio(mbuf.ptr() + y * blockCountX, blockCountX, 1, ratioI);
		imposeRatio(mbuf.ptr() + ((y + 1) * blockCountX) - 1, blockCountX, - 1, ratioI);
	}

	for (int32_t x = 0; x < blockCountX; x++)
	{
		imposeRatio(mbuf.ptr() + x, blockCountY, blockCountX, ratioI);
		imposeRatio(mbuf.ptr() + x + (blockCountX * (blockCountY - 1)), blockCountY, -blockCountX, ratioI);
	}

    // For each pixel, scale the pixel RGB values based on chosen M.
    for (int32_t y = 0; y < image->getHeight(); y++)
	{
		uint8_t* s = imageData + 4 * y * image->getWidth();
		uint8_t* d = mbuf.ptr() + ((y + leftOverY) / blockDimY) * blockCountX;

		int32_t ctr = 0;
		int32_t mm = *d++;
		for (int32_t x = 0; x < image->getWidth(); x++)
		{
			uint8_t m = s[4 * x + 3];
			if (m != mm)
			{
				uint8_t r = s[4 * x];
				uint8_t g = s[4 * x + 1];
				uint8_t b = s[4 * x + 2];
				s[4 * x]     = (r * m + (mm >> 1)) / mm;
				s[4 * x + 1] = (g * m + (mm >> 1)) / mm;
				s[4 * x + 2] = (b * m + (mm >> 1)) / mm;
				s[4 * x + 3] = mm;
			}

			if (++ctr == blockDimX)
			{
				ctr = 0;
				mm = *d++;
			}
		}
    }
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.EncodeRGBM", EncodeRGBM, IImageFilter)

EncodeRGBM::EncodeRGBM(float range, int32_t blockSizeX, int32_t blockSizeY, float blockFilterRatio)
:	m_range(range)
,	m_blockSizeX(blockSizeX)
,	m_blockSizeY(blockSizeY)
,	m_blockFilterRatio(blockFilterRatio)
{
}

void EncodeRGBM::apply(Image* image) const
{
	const Scalar c_rangeDenom = 1.0_simd / Scalar(m_range);

	Color4f cl;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, cl);
			cl *= c_rangeDenom;

			float a = clamp< float >( max( max( cl.getRed(), cl.getGreen() ), cl.getBlue() ), 0.0f, 1.0f );
			if (a > FUZZY_EPSILON)
			{
				a = std::ceil(a * 255.0f) / 255.0f;
				cl /= Scalar(a);
				cl.setAlpha(Scalar(a));
			}
			else
				cl.set(0.0f, 0.0f, 0.0f, 0.0f);

			image->setPixelUnsafe(x, y, cl);
		}
	}

	if (m_blockSizeX > 0 && m_blockSizeY > 0)
	{
		image->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		prepareForBlockCompression(image, m_blockSizeX, m_blockSizeY, m_blockFilterRatio);
	}
}

}
