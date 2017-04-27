/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <rg_etc1.h>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/EtcCompressor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.EtcCompressor", EtcCompressor, ICompressor)

bool EtcCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	log::info << L"Compressing and writing mip(s)..." << Endl;

	int32_t mipCount = int32_t(mipImages.size());

	int32_t w = mipImages[0]->getWidth();
	int32_t h = mipImages[0]->getHeight();

	int32_t maxByteSize = getTextureMipPitch(TfETC1, w, h, 0);
	AutoPtr< uint8_t > compressedData(new uint8_t [maxByteSize]);

	rg_etc1::etc1_pack_params params;
	rg_etc1::pack_etc1_block_init();

	for (int32_t i = 0; i < mipCount; ++i)
	{
		int32_t byteSize = getTextureMipPitch(TfETC1, w, h, i);
		T_ASSERT (byteSize <= maxByteSize);

		Ref< drawing::Image > mipImage = mipImages[i];
		uint8_t* wp = compressedData.ptr();

		for (int32_t y = 0; y < mipImage->getHeight(); y += 4)
		{
			for (int32_t x = 0; x < mipImage->getWidth(); x += 4)
			{
				uint8_t source[4 * 4 * 4];
				uint8_t* sp = source;

				for (int32_t iy = 0; iy < 4; ++iy)
				{
					for (int32_t ix = 0; ix < 4; ++ix)
					{
						Color4f tmp;
						mipImage->getPixel(x, y, tmp);

						*sp++ = uint8_t(tmp.getRed() * 255);
						*sp++ = uint8_t(tmp.getGreen() * 255);
						*sp++ = uint8_t(tmp.getBlue() * 255);
						*sp++ = 255;
					}
				}

				rg_etc1::pack_etc1_block(
					wp,
					(const unsigned int *)source,
					params
				);

				wp += 8;
			}
		}

		if (writer.write(compressedData.ptr(), byteSize, 1) != byteSize)
			return false;
	}

	log::info << L"All mip(s) written" << Endl;
	return true;
}

	}
}
