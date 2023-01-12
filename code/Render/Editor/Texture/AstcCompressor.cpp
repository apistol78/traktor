/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <astcenc.h>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/AstcCompressor.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AstcCompressor", AstcCompressor, ICompressor)

bool AstcCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	astcenc_config config;
	astcenc_error result;

	astcenc_profile profile = ASTCENC_PRF_LDR_SRGB;
	if (textureFormat >= TfASTC4x4F)
		profile = ASTCENC_PRF_HDR;

	switch (textureFormat)
	{
	case TfASTC4x4:
	case TfASTC4x4F:
		result = astcenc_config_init(
			profile,
			4,	// block x
			4,	// block y
			1,	// block z,
			ASTCENC_PRE_MEDIUM,
			0,
			&config
		);
		break;

	case TfASTC8x8:
	case TfASTC8x8F:
		result = astcenc_config_init(
			profile,
			8,	// block x
			8,	// block y
			1,	// block z,
			ASTCENC_PRE_MEDIUM,
			0,
			&config
		);
		break;

	case TfASTC10x10:
	case TfASTC10x10F:
		result = astcenc_config_init(
			profile,
			10,	// block x
			10,	// block y
			1,	// block z,
			ASTCENC_PRE_MEDIUM,
			0,
			&config
		);
		break;

	case TfASTC12x12:
	case TfASTC12x12F:
		result = astcenc_config_init(
			profile,
			12,	// block x
			12,	// block y
			1,	// block z,
			ASTCENC_PRE_MEDIUM,
			0,
			&config
		);
		break;

	default:
		log::error << L"Unable to compress using ASTC; invalid texture format." << Endl;
		return false;
	}
	
	if (result != ASTCENC_SUCCESS)
	{
		log::error << L"Unable to compress using ASTC; config failed." << Endl;
		return false;
	}

	astcenc_context* context = nullptr;
	result = astcenc_context_alloc(&config, 1, &context);
	if (result != ASTCENC_SUCCESS)
	{
		log::error << L"Unable to compress using ASTC; failed to allocate context." << Endl;
		return false;
	}

	for (size_t i = 0; i < mipImages.size(); ++i)
	{
		const uint8_t* sourceImageData = (const uint8_t*)mipImages[i]->getData();
		T_FATAL_ASSERT(sourceImageData != nullptr);

		uint32_t pitch = mipImages[i]->getWidth() * mipImages[i]->getPixelFormat().getByteSize();

		AutoArrayPtr< const uint8_t* > imageRows(new const uint8_t* [mipImages[i]->getHeight()]);
		for (uint32_t y = 0; y < mipImages[i]->getHeight(); ++y)
			imageRows[y] = sourceImageData + pitch * y;

		void* data = (void*)imageRows.c_ptr();

		astcenc_image image;
		image.dim_x = mipImages[i]->getWidth();
		image.dim_y = mipImages[i]->getHeight();
		image.dim_z = 1;

		if (mipImages[i]->getPixelFormat().isFloatPoint())
		{
			if (mipImages[i]->getPixelFormat().getRedBits() == 32)
				image.data_type = ASTCENC_TYPE_F32;
			else if (mipImages[i]->getPixelFormat().getRedBits() == 16)
				image.data_type = ASTCENC_TYPE_F16;
			else
			{
				log::error << L"Unable to compress using ASTC; unsupported number of float point bits." << Endl;
				return false;
			}
		}
		else
			image.data_type = ASTCENC_TYPE_U8;

		image.data = &data;

		astcenc_swizzle swizzle;
		swizzle.r = ASTCENC_SWZ_R;
		swizzle.g = ASTCENC_SWZ_G;
		swizzle.b = ASTCENC_SWZ_B;
		swizzle.a = ASTCENC_SWZ_A;

		size_t imageSize = getTextureMipPitch(textureFormat, image.dim_x, image.dim_y);
		AutoArrayPtr< uint8_t > imageData(new uint8_t [imageSize]);

		result = astcenc_compress_image(context, &image, &swizzle, imageData.ptr(), imageSize, 0);
		if (result != ASTCENC_SUCCESS)
		{
			log::error << L"Unable to compress using ASTC; failed to compress image." << Endl;
			return false;
		}

		result = astcenc_compress_reset(context);
		if (result != ASTCENC_SUCCESS)
		{
			log::error << L"Unable to compress using ASTC; failed to reset context." << Endl;
			return false;
		}

		if (writer.write(imageData.c_ptr(), imageSize, 1) != imageSize)
		{
			log::error << L"Unable to compress using ASTC; failed to write image data to file." << Endl;
			return false;
		}
	}

	astcenc_context_free(context);
    return true;
}

}
