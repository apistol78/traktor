/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/Bc6hCompressor.h"

#define BC6H_ENC_IMPLEMENTATION
#include "bc6h_enc.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Bc6hCompressor", Bc6hCompressor, ICompressor)

bool Bc6hCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	AutoPtr< uint8_t > compressedData[64];
	Ref< Job > jobs[64];

	const int32_t mipCount = (int32_t)mipImages.size();
	for (int32_t i = 0; i < mipCount; ++i)
	{
		const drawing::Image* mipImage = mipImages[i];
		const int32_t byteSize = getTextureMipPitch(textureFormat, mipImage->getWidth(), mipImage->getHeight());

		compressedData[i].reset(new uint8_t[byteSize]);
		uint8_t* compressedDataPtr = compressedData[i].ptr();

		jobs[i] = JobManager::getInstance().add([=]() {
			Color4f tmp;

			uint8_t* wp = compressedDataPtr;
			for (int32_t y = 0; y < mipImage->getHeight(); y += 4)
			{
				for (int32_t x = 0; x < mipImage->getWidth(); x += 4)
				{
					float T_MATH_ALIGN16 source[4 * 4 * 4];
					float* sp = source;

					for (int32_t iy = 0; iy < 4; ++iy)
					{
						for (int32_t ix = 0; ix < 4; ++ix)
						{
							mipImage->getPixel(x + ix, y + iy, tmp);
							tmp.storeAligned(sp);
							sp += 4;
						}
					}

					if (textureFormat == TfBC6HU)
						bc6h_enc::EncodeBC6HU(wp, source);
					else if (textureFormat == TfBC6HS)
						bc6h_enc::EncodeBC6HS(wp, source);

					wp += 16;
				}
			}
		});
	}

	for (int32_t i = 0; i < mipCount; ++i)
	{
		if (!jobs[i]->wait())
			return false;

		const drawing::Image* mipImage = mipImages[i];
		const int32_t byteSize = getTextureMipPitch(textureFormat, mipImage->getWidth(), mipImage->getHeight());

		if (writer.write(compressedData[i].ptr(), byteSize, 1) != byteSize)
			return false;
	}

	return true;
}

}
